// ghaerosmith - the crowd's claps on star power follow the audio calibration.
//
// While a player's star power is on, the crowd claps on the beat. As in GH Metallica (GHMetalRecomp's src/crowd.cpp,
// docs/crowd-claps.md), the game plays the claps from a script spawned for each fretbar: the fretbar's handler (script
// 0x5E4AD8C0) waits 0.18 s, then plays a clap (SoundEvent) when a player has star_power_used. The song's setup
// (script 0x2324FE5A) registers it on the song's fretbars with an offset that takes the player's calibration off, from
// the global tags user_options: ($0x582DBCE4 * 1000 + $0xC79B27C1) - audio_offset + video_offset ms. The fretbar events
// come as late as that offset is below its base, and the 0.18 s does not take it back: in GH Metallica, Gustavo heard
// the claps 104 ms after the music's beat with 104 ms of audio calibration, as on the console.
//
// Here the calibration is not kept in a script global: the setup keeps it in a local variable and passes the offset to
// the fretbar iterator (fretbar_iterator_CFunc, sub_82116B20), which stores it in its own state. A hook there records the
// offset the handler was registered with; the handler's wait then takes off how far it is below its base, read from the
// script globals.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>

REXCVAR_DEFINE_BOOL(ghaerosmith_crowd_claps_fix, true, "GHAerosmith",
                    "The crowd's claps on star power keep time with the music whatever the audio and video "
                    "calibration (the game claps late by the calibration); false: as on the console");

namespace {

constexpr uint32_t kFretbarHandlerScript = 0x5E4AD8C0;
constexpr uint32_t kGlobalsTable = 0x82B94030;  // the script globals' hash table (a pointer), sub_82145018's
// The handler's offset is ($kOffsetSeconds * 1000 + $kOffsetBase) ms less the calibration (script 0x2324FE5A).
constexpr uint32_t kOffsetSeconds = 0x582DBCE4;
constexpr uint32_t kOffsetBase = 0xC79B27C1;

// The offset the fretbar iterator of the handler was set up with (ms), NaN before the first song.
std::atomic<float> g_registered_offset{NAN};

const uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

float LoadFloat(const uint8_t* base, uint32_t address) {
  const uint32_t bits = Load32(base, address);
  float value;
  std::memcpy(&value, &bits, 4);
  return value;
}

// A script global's number, as sub_82145018 reads it: buckets by the key's low 15 bits, entries {+2 type, +4 key,
// +12 value, +16 next}; type 1 an integer, 2 a float, 26 another global's key.
std::optional<double> ScriptGlobal(const uint8_t* base, uint32_t key) {
  const uint32_t table = Load32(base, kGlobalsTable);
  if (!table) {
    return std::nullopt;
  }
  for (int alias = 0; alias < 4; ++alias) {
    uint32_t entry = Load32(base, table + ((key & 0x7FFF) << 2));
    for (int length = 0; entry && length < 4096 && Load32(base, entry + 4) != key; ++length) {
      entry = Load32(base, entry + 16);
    }
    if (!entry) {
      return std::nullopt;
    }
    const uint8_t type = base[entry + 2];
    const uint32_t value = Load32(base, entry + 12);
    if (type == 1) {
      return double(int32_t(value));
    }
    if (type == 2) {
      float f;
      std::memcpy(&f, &value, 4);
      return double(f);
    }
    if (type != 26) {
      return std::nullopt;
    }
    key = value;
  }
  return std::nullopt;
}

uint32_t ScriptName(const uint8_t* base, uint32_t script) { return script ? Load32(base, script + 216) : 0; }

}  // namespace

// [[midasm_hook]] 0x82116CB4 (fretbar_iterator_CFunc, sub_82116B20, setting up its state): before "stw r11,24(r31)",
// r11 = the script it will spawn on each fretbar, r31 = its state, the offset (float, ms) already at +20.
void GHAerosmithFretbarIteratorHook(PPCRegister& r11, PPCRegister& r31) {
  const uint8_t* base = GuestBase();
  if (base && r11.u32 == kFretbarHandlerScript) {
    g_registered_offset = LoadFloat(base, r31.u32 + 20);
  }
}

// [[midasm_hook]] 0x8225FF60 (the script function Wait, sub_8225FD10, its seconds branch): r4 = the wait in
// milliseconds, r30 = the script. The fretbar handler's wait is shortened by the calibration.
void GHAerosmithCrowdBeatWaitHook(PPCRegister& r4, PPCRegister& r30) {
  if (!REXCVAR_GET(ghaerosmith_crowd_claps_fix)) {
    return;
  }
  const uint8_t* base = GuestBase();
  if (!base || ScriptName(base, r30.u32) != kFretbarHandlerScript) {
    return;
  }
  const float registered = g_registered_offset;
  const auto seconds = ScriptGlobal(base, kOffsetSeconds);
  const auto offset_base = ScriptGlobal(base, kOffsetBase);
  if (std::isnan(registered) || !seconds || !offset_base) {
    return;
  }
  const int32_t calibration = int32_t(std::lround(*seconds * 1000.0 + *offset_base - registered));
  const int32_t wait = int32_t(r4.u32);
  // A video offset larger than the audio one registers the handler early: its wait is then longer (up to a second).
  r4.u64 = uint64_t(uint32_t(std::clamp(wait - calibration, 0, wait + 1000)));
  static int32_t logged = INT32_MIN;
  if (calibration != logged) {
    logged = calibration;
    REXLOG_INFO("[crowd] the claps' wait {} ms less the calibration {} ms (registered at {} ms of {} * 1000 + {})",
                wait, calibration, registered, *seconds, *offset_base);
  }
}
