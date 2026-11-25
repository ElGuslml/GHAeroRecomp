// ghaerosmith - the game's script globals changed in guest memory (see script_globals.h).
//
// Guitar Hero III's interpreter keeps every global in a table: the pointer at 0x82B94030 (sub_8219BF30, the "$name"
// resolve, reads it: lis r11,-32071; lwz r6,16432(r11)) to 32768 buckets, indexed by the key's low 15 bits, of nodes (+2
// type, +4 key, +12 value, +16 next; type 26 names another global), as GH5's and GH Warriors of Rock's (their
// src/script_globals.cpp). A struct (type 10) has its first component's pointer at +4 (sub_82145018 reads it so); a
// component is +1 its type (the low 7 bits; the bit 0x80 is something else, kept), +4 name, +8 value, +12 next: the
// one difference from GH5, whose component has its type at +2. An array (type 12) is +2 the elements' type, +4 their
// count, +8 the value when there is one, else a pointer to them (the disc's qb data: highway_normal is 00 01 01 00,
// 4, its ints; rowheight 00 01 02 00, 1153, its floats). Integers are type 1, floats type 2, names (checksums) 13.
//
// A global's data written in place reaches the scripts the interpreter runs and the compiled ones alike. The game loads
// every global at the start and keeps it; the changes are written at the first "$" read of any global (the boot's
// scripts), before the menus or a song read them.

#include "script_globals.h"

#include <rex/logging.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace ghaerosmith::script {
namespace {

constexpr uint32_t kGlobalTable = 0x82B94030;
constexpr uint8_t kTypeMask = 0x7F;  // a component's type: the low 7 bits of its byte at +1
constexpr uint8_t kTypeInteger = 1;
constexpr uint8_t kTypeFloat = 2;
constexpr uint8_t kTypeVector2 = 5;
constexpr uint8_t kTypeVector3 = 6;
constexpr uint8_t kTypeStruct = 10;
constexpr uint8_t kTypeArray = 12;
constexpr uint8_t kTypeName = 13;
constexpr uint8_t kTypeAlias = 26;

uint8_t* Base() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

// Whether the host has the guest's bytes there (a pointer followed from the game's data, checked before it is read).
bool Readable(uint32_t address, uint32_t size) {
  uint8_t* base = Base();
  if (!base || address < 0x10000 || address + size < address) {
    return false;
  }
#ifdef _WIN32
  uint8_t* p = base + address;
  uint8_t* end = p + size;
  while (p < end) {
    MEMORY_BASIC_INFORMATION info{};
    if (!VirtualQuery(p, &info, sizeof(info)) || info.State != MEM_COMMIT || !info.Protect ||
        (info.Protect & (PAGE_NOACCESS | PAGE_GUARD))) {
      return false;
    }
    p = static_cast<uint8_t*>(info.BaseAddress) + info.RegionSize;
  }
#endif
  return true;
}

uint32_t Load32(uint32_t address) {
  uint8_t* base = Base();
  if (!base || !Readable(address, 4)) {
    return 0;
  }
  uint32_t v;
  std::memcpy(&v, base + address, 4);
  return __builtin_bswap32(v);
}

void Store32(uint32_t address, uint32_t value) {
  uint8_t* base = Base();
  if (!base || !Readable(address, 4)) {
    return;
  }
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

uint8_t Load8(uint32_t address) {
  uint8_t* base = Base();
  return base && Readable(address, 1) ? base[address] : 0;
}

struct Step {
  enum Kind { kField, kIndex, kEveryStruct, kEveryElement } kind;
  uint32_t key = 0;
  uint32_t index = 0;
};

struct Change {
  std::string path;
  uint32_t root = 0;
  std::vector<Step> steps;
  bool scale = false;
  double value = 0.0;
  bool applied = false;
};

struct Read {
  uint8_t type = kTypeInteger;  // an integer, or a name (13)
  uint32_t value = 0;
};

std::mutex g_mutex;
std::vector<Change> g_changes;
std::map<uint64_t, Read> g_reads;  // (script << 32 | global) -> the value read
std::atomic<bool> g_pending{false};
std::atomic<bool> g_has_reads{false};

bool Parse(const std::string& path, Change& change) {
  size_t i = 0;
  auto ident = [&]() {
    size_t start = i;
    while (i < path.size() && (std::isalnum(uint8_t(path[i])) || path[i] == '_' || path[i] == '*')) {
      ++i;
    }
    return path.substr(start, i - start);
  };
  std::string root = ident();
  if (root.empty() || root == "*") {
    return false;
  }
  change.root = QbKey(root);
  while (i < path.size()) {
    if (path[i] == '.') {
      ++i;
      std::string field = ident();
      if (field == "*") {
        change.steps.push_back({Step::kEveryStruct});
      } else if (!field.empty()) {
        change.steps.push_back({Step::kField, QbKey(field)});
      } else {
        return false;
      }
    } else if (path[i] == '[') {
      size_t close = path.find(']', i);
      if (close == std::string::npos || close == i + 1) {
        return false;
      }
      Step step{Step::kIndex};
      if (path[i + 1] == '*') {
        step.kind = Step::kEveryElement;
      } else {
        step.index = uint32_t(std::strtoul(path.c_str() + i + 1, nullptr, 10));
      }
      change.steps.push_back(step);
      i = close + 1;
    } else {
      return false;
    }
  }
  return true;
}

void Add(std::string path, double value, bool scale) {
  Change change;
  change.path = std::move(path);
  change.scale = scale;
  change.value = value;
  if (!Parse(change.path, change)) {
    REXLOG_ERROR("[script globals] '{}': not a path", change.path);
    return;
  }
  std::lock_guard lock(g_mutex);
  g_changes.push_back(std::move(change));
  g_pending = true;
}

// The global's node, through the globals that name another.
uint32_t FindNode(uint32_t key) {
  for (int hops = 0; hops < 4; ++hops) {
    uint32_t table = Load32(kGlobalTable);
    if (!table) {
      return 0;
    }
    uint32_t node = Load32(table + (key & 0x7FFF) * 4);
    for (int guard = 0; node && Load32(node + 4) != key && guard < 100000; ++guard) {
      node = Load32(node + 16);
    }
    if (!node || Load8(node + 2) != kTypeAlias) {
      return node;
    }
    key = Load32(node + 12);
  }
  return 0;
}

uint32_t FirstComponent(uint32_t s) {
  uint32_t first = Load32(s + 4);
  return first && Readable(first, 16) ? first : 0;
}

// Each value the steps reach from a value of `type` held at `slot`: f(slot, type).
template <typename F>
void Walk(uint8_t type, uint32_t slot, const std::vector<Step>& steps, size_t at, const std::string& path, F&& f) {
  if (at == steps.size()) {
    f(slot, type);
    return;
  }
  const Step& step = steps[at];
  uint32_t value = Load32(slot);
  if (step.kind == Step::kIndex && (type == kTypeVector2 || type == kTypeVector3)) {
    // A vector: a 4-byte header, then its floats.
    if (step.index >= (type == kTypeVector2 ? 2u : 3u)) {
      REXLOG_ERROR("[script globals] {}: no component {} of a vector", path, step.index);
      return;
    }
    Walk(kTypeFloat, value + 4 + step.index * 4, steps, at + 1, path, f);
    return;
  }
  if (step.kind == Step::kIndex || step.kind == Step::kEveryElement) {
    uint32_t count = type == kTypeArray ? Load32(value + 4) : 0;
    if (step.kind == Step::kIndex && step.index >= count) {
      REXLOG_ERROR("[script globals] {}: no element {} (a type {} of {})", path, step.index, type, count);
      return;
    }
    const uint32_t first = step.kind == Step::kIndex ? step.index : 0;
    const uint32_t last = step.kind == Step::kIndex ? step.index + 1 : std::min<uint32_t>(count, 4096);
    for (uint32_t index = first; index < last; ++index) {
      uint32_t element = count == 1 ? value + 8 : Load32(value + 8) + index * 4;
      Walk(Load8(value + 2), element, steps, at + 1, path, f);
    }
    return;
  }
  if (type != kTypeStruct) {
    REXLOG_ERROR("[script globals] {}: a field of a type {}", path, type);
    return;
  }
  bool found = false;
  int guard = 0;
  for (uint32_t c = FirstComponent(value); c && guard < 512; c = Load32(c + 12), ++guard) {
    uint8_t ctype = Load8(c + 1) & kTypeMask;
    if (step.kind == Step::kEveryStruct ? ctype == kTypeStruct : Load32(c + 4) == step.key) {
      found = true;
      Walk(ctype, c + 8, steps, at + 1, path, f);
      if (step.kind == Step::kField) {
        break;
      }
    }
  }
  if (!found) {
    REXLOG_ERROR("[script globals] {}: field not found", path);
  }
}

float AsFloat(uint32_t bits) {
  float f;
  std::memcpy(&f, &bits, 4);
  return f;
}

uint32_t FloatBits(float f) {
  uint32_t bits;
  std::memcpy(&bits, &f, 4);
  return bits;
}

void ApplyChanges() {
  for (Change& change : g_changes) {
    if (change.applied) {
      continue;
    }
    uint32_t node = FindNode(change.root);
    if (!node) {
      continue;
    }
    change.applied = true;
    int count = 0;
    Walk(Load8(node + 2), node + 12, change.steps, 0, change.path, [&](uint32_t slot, uint8_t type) {
      if (type == kTypeFloat) {
        float before = AsFloat(Load32(slot));
        float after = float(change.scale ? before * change.value : change.value);
        Store32(slot, FloatBits(after));
        REXLOG_INFO("[script globals] {}: {} -> {}", change.path, before, after);
        ++count;
      } else if (type == kTypeInteger) {
        int32_t before = int32_t(Load32(slot));
        int32_t after = int32_t(std::lround(change.scale ? before * change.value : change.value));
        Store32(slot, uint32_t(after));
        REXLOG_INFO("[script globals] {}: {} -> {}", change.path, before, after);
        ++count;
      } else {
        REXLOG_ERROR("[script globals] {}: a type {}, left as it is", change.path, type);
      }
    });
    if (!count) {
      REXLOG_ERROR("[script globals] {}: nothing changed", change.path);
    }
  }
  g_pending = std::any_of(g_changes.begin(), g_changes.end(), [](const Change& c) { return !c.applied; });
}

void AddRead(std::string_view script, std::string_view global, Read read) {
  std::lock_guard lock(g_mutex);
  g_reads[(uint64_t(QbKey(script)) << 32) | QbKey(global)] = read;
  g_has_reads = true;
}

}  // namespace

uint32_t QbKey(std::string_view name) {
  uint32_t crc = 0xFFFFFFFF;
  for (char c : name) {
    crc ^= uint8_t(std::tolower(uint8_t(c)));
    for (int i = 0; i < 8; ++i) {
      crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
    }
  }
  return crc;
}

void Set(std::string path, double value) { Add(std::move(path), value, false); }

void Scale(std::string path, double factor) { Add(std::move(path), factor, true); }

void ReadAs(std::string_view script, std::string_view global, int32_t value) {
  AddRead(script, global, {kTypeInteger, uint32_t(value)});
}

void ReadAsName(std::string_view script, std::string_view global, std::string_view name) {
  AddRead(script, global, {kTypeName, QbKey(name)});
}

bool Pending() { return g_pending.load(std::memory_order_relaxed); }

bool NeedsScriptNames() { return g_has_reads.load(std::memory_order_relaxed); }

bool OnResolve(uint8_t* base, uint32_t component, uint32_t global, uint32_t script) {
  // At any global's read, not only the changed one's: a "$name" inside a struct literal (rgba = $highway_normal in
  // setup_highway's CreateScreenElement) is resolved by the struct's own getters (sub_82144A30, sub_821448E0, ...), not
  // here. The scripts are all loaded at the start, so the first read finds every global; one not found stays pending.
  if (g_pending.load(std::memory_order_relaxed)) {
    std::lock_guard lock(g_mutex);
    if (g_pending) {
      ApplyChanges();
    }
  }
  if (!script || !g_has_reads.load(std::memory_order_relaxed)) {
    return false;
  }
  // Without the lock: the reads are all added before the hooks first run (src/mods.cpp's Configure, from
  // src/debug_menu.cpp), and never after.
  auto it = g_reads.find((uint64_t(script) << 32) | global);
  if (it == g_reads.end()) {
    return false;
  }
  // As sub_8219BF30 writes a value: the type in the low 7 bits of +1 (the bit 0x80 kept), the value at +8.
  base[component + 1] = uint8_t((base[component + 1] & ~kTypeMask) | it->second.type);
  uint32_t bits = __builtin_bswap32(it->second.value);
  std::memcpy(base + component + 8, &bits, 4);
  return true;
}

}  // namespace ghaerosmith::script
