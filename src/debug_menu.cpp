// ghaerosmith - the game's own debug menu, offered in the main menu and the pause menu.
//
// The game ships Neversoft's debug menu (scripts\guitar\guitar_debug_menu.qb, create_debugging_menu). The main menu
// (script create_main_menu) and the pause menu (create_pause_menu) add an item that opens it when the script global
// enable_button_cheats is 1. bootup_menu_flow.qb defines it as 0 and guitar_startup sets it again (and with 1 would run
// LaunchViewer). The same global also turns on development shortcuts that change the save: setlist_debug_unlockall and
// setlist_debug_completesong (the setlist), store_debug_givebandcash (the store), and the training menu's tutorials for
// a gamepad (choose_training_mode, switch_training_mode, create_select_practice_mode_menu: "IsPS3 and").
//
// So the global stays 0 and only those two scripts read it as 1, as in GH Metallica (GHMetalRecomp's
// src/debug_menu.cpp), whose interpreter this one is an older version of. A "$name" in an expression (tokens 0x4B
// 0x16 key) becomes a component of type 13 (the low 7 bits at +1; 26 for a global naming another) with the name's key at
// +8, which sub_8219BF30 replaces by the global's value (an integer: type 1, the value at +8; the bit 0x80 at +1 kept).
// The interpreter's statement (sub_82179D38, r31 = the script) evaluates its expressions at three places: twice with
// sub_822FF180 (an expression in parentheses, as the menus' "if ($enable_button_cheats = 1)"), once with sub_8219BF30
// itself (a component of type 26). Hooks there keep, per thread, the scripts whose expressions are being evaluated;
// the running script's name is at +216 of it (as src/framerate.cpp and src/crowd.cpp read it).
//
// The same hooks serve the mods (src/mods.cpp, src/script_globals.cpp, as GH5's): a global changed in place at its
// first read, and globals read as another value by one script.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <set>

#include "highway_images.h"
#include "mods.h"
#include "script_globals.h"

REXCVAR_DEFINE_BOOL(ghaerosmith_debug_menu, false, "GHAerosmith",
                    "The game's own debug menu (Neversoft's): an item in the main menu and the pause menu that opens "
                    "it, without the development shortcuts that change the save; false = as on the console");

namespace {

constexpr uint32_t kEnableButtonCheats = 0x2AF92804;  // the script global enable_button_cheats
constexpr uint32_t kCreateMainMenu = 0x43634516;      // the main menu's items
constexpr uint32_t kCreatePauseMenu = 0x3562432C;     // the pause menu's items
constexpr uint32_t kSetupHighway = 0x23D93A37;        // setup_highway: a song's highways made (src/highway_images.cpp)
constexpr uint8_t kTypeMask = 0x7F;
constexpr uint8_t kTypeInteger = 1;
constexpr uint8_t kTypeName = 13;
constexpr uint8_t kTypeGlobalAlias = 26;
constexpr uint32_t kScriptName = 216;  // the running script's name in the interpreter's script object

bool DebugMenu() {
  static const bool enabled = REXCVAR_GET(ghaerosmith_debug_menu);
  return enabled;
}

// Whether a mod reads a global as another value in some script (src/mods.cpp, configured at the first call).
bool ModReads() {
  static const bool reads = (ghaerosmith::mods::Configure(), ghaerosmith::script::NeedsScriptNames());
  return reads;
}

// Whether a random highway picture changes at each song (src/highway_images.cpp, configured with the mods).
bool WatchesSongs() {
  static const bool watches = (ModReads(), ghaerosmith::highway::Random());
  return watches;
}

// Whether the evaluation hooks must know each read's script. ModReads first, so the mods are configured before
// script::Pending() is asked.
bool Active() { return ModReads() || DebugMenu() || WatchesSongs(); }

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

void Store32(uint8_t* base, uint32_t address, uint32_t value) {
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

// The scripts whose expressions this thread is evaluating, innermost last (an expression can run another script).
struct Evaluations {
  std::array<uint32_t, 64> scripts{};
  uint32_t depth = 0;
};
thread_local Evaluations t_evaluations;

void LogOnce(uint32_t script, bool shown) {
  static std::mutex mutex;
  static std::set<uint32_t> logged;
  std::lock_guard lock(mutex);
  if (logged.size() < 32 && logged.insert(script).second) {
    if (shown) {
      REXLOG_INFO("[debug menu] script {:08X} offers the debug menu (enable_button_cheats read as 1)", script);
    } else {
      REXLOG_INFO("[debug menu] script {:08X} reads enable_button_cheats: left at 0 (a development shortcut)", script);
    }
  }
}

}  // namespace

// [[midasm_hook]] 0x82179F14, 0x8217A1F0 (sub_82179D38, at each bl sub_822FF180) and 0x8217A240 (its bl
// sub_8219BF30): r31 = the script.
void GHAerosmithScriptEvalEnterHook(PPCRegister& r31) {
  if (!Active()) {
    return;
  }
  Evaluations& e = t_evaluations;
  if (e.depth < e.scripts.size()) {
    e.scripts[e.depth] = r31.u32;
  }
  ++e.depth;
  if (WatchesSongs()) {
    if (const uint8_t* base = GuestBase(); base && r31.u32 && Load32(base, r31.u32 + kScriptName) == kSetupHighway) {
      ghaerosmith::highway::OnHighwaySetup();
    }
  }
}

// [[midasm_hook]] 0x82179F18, 0x8217A1F4 and 0x8217A244 (sub_82179D38, after each of those calls).
void GHAerosmithScriptEvalLeaveHook() {
  if (!Active()) {
    return;
  }
  Evaluations& e = t_evaluations;
  if (e.depth > 0) {
    --e.depth;
  }
}

// [[midasm_hook]] 0x8219BF30 (the entry of sub_8219BF30: a "$name" component resolved to the global's value),
// return_on_true: r3 = the component. A mod's change of that global lands first (src/script_globals.cpp); a mod's read
// as another value, or enable_button_cheats read by the menus' scripts as the integer 1, writes the component.
bool GHAerosmithGlobalResolveHook(PPCRegister& r3) {
  const bool active = Active();
  if (!active && !ghaerosmith::script::Pending()) {
    return false;
  }
  uint8_t* base = GuestBase();
  const uint32_t component = r3.u32;
  if (!base || !component) {
    return false;
  }
  const uint8_t flags = base[component + 1];
  const uint8_t type = flags & kTypeMask;
  if (type != kTypeName && type != kTypeGlobalAlias) {
    return false;
  }
  const Evaluations& e = t_evaluations;
  // The reading script: known only while the evaluation hooks run (Active), and not for compiled scripts.
  const bool known = active && e.depth > 0 && e.depth <= e.scripts.size() && e.scripts[e.depth - 1];
  const uint32_t global = Load32(base, component + 8);
  const uint32_t script = known ? Load32(base, e.scripts[e.depth - 1] + kScriptName) : 0;
  if (ghaerosmith::script::OnResolve(base, component, global, script)) {
    return true;
  }
  if (!known || !DebugMenu() || global != kEnableButtonCheats) {
    return false;
  }
  const bool menu = script == kCreateMainMenu || script == kCreatePauseMenu;
  LogOnce(script, menu);
  if (!menu) {
    return false;
  }
  base[component + 1] = uint8_t((flags & ~kTypeMask) | kTypeInteger);
  Store32(base, component + 8, 1);
  return true;
}
