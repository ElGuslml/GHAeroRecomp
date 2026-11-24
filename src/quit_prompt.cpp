// ghaerosmith - quitting from the title screen and the main menu.
//
// The game has no way out of itself (on the console the dashboard was). On the title screen ("JUST PUSH ANY BUTTON TO
// PLAY...") and in the main menu (Career, Quickplay, Multiplayer...), B now asks whether to quit the game: the SDK's
// quit prompt (rex/ui/overlay/quit_prompt.h, on the recomp branch), a dialog in the game's language where A quits (the
// window closes, as with its close button) and B cancels. The game does not see those presses. As in GH Metallica
// (GHMetalRecomp's src/quit_prompt.cpp, docs/quit-prompt.md).
//
// The game's screens are made of screen elements that its scripts create and destroy (CreateScreenElement,
// sub_82173878; DestroyScreenElement, sub_82174848), each with an id, the checksum of its name. Two of them are on one
// of these screens only (found with a temporary probe of the elements each frame and the scripts,
// DATA/COMPRESSED/PAK/qb.pab.xen):
// - 0x9884EF17, the title screen's container, the parent of its "JUST PUSH ANY BUTTON TO PLAY..." text (script
//   0x47794E43);
// - 0x20D40671, main_menu_text_container, from the main menu until a choice leaves it.
// The engine finds an element by id in a hash table (sub_820F3B48, the table object at [0x82A48430], which
// ScreenElementExists sub_82174720 uses); the hook reads the same table once per frame on the main thread, which runs
// the scripts, and offers the prompt while either element exists and no warning popup does (0xA19EEF0E,
// popup_warning_container; 0x48B4C386, quit_warning_container: a message box over the menu takes B itself).
// quit_prompt = false (an SDK cvar) disables it.

#include "players.h"

#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>
#include <rex/ui/overlay/quit_prompt.h>

#include <cstdint>
#include <cstring>

namespace {

constexpr uint32_t kElementTable = 0x82A48430;  // the screen elements' table object (a pointer)
constexpr uint32_t kTitleContainer = 0x9884EF17;
constexpr uint32_t kMainMenuContainer = 0x20D40671;    // QBKey("main_menu_text_container")
constexpr uint32_t kPopupWarning = 0xA19EEF0E;         // QBKey("popup_warning_container")
constexpr uint32_t kQuitWarning = 0x48B4C386;          // QBKey("quit_warning_container")

const uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

// sub_820F3B48: the element with this id, or 0. The table object holds two hash tables (+8, +12), each
// {uint32 bits, entry* buckets, ..., uint8 keep_empty at +21}; an entry is {uint32 key, uint32 value, entry* next}
// and the chain starts at buckets[id & ((1 << bits) - 1)].
uint32_t FindElement(const uint8_t* base, uint32_t id) {
  const uint32_t table = Load32(base, kElementTable);
  if (!table) {
    return 0;
  }
  for (uint32_t offset : {8u, 12u}) {
    const uint32_t hash = Load32(base, table + offset);
    if (!hash) {
      continue;
    }
    const uint32_t bits = Load32(base, hash);
    const uint32_t buckets = Load32(base, hash + 4);
    if (!buckets || bits > 24) {
      continue;
    }
    uint32_t entry = buckets + (id & ((1u << bits) - 1)) * 12;
    for (int length = 0; entry && length < 4096; ++length) {
      if (Load32(base, entry) == id) {
        if (const uint32_t value = Load32(base, entry + 4)) {
          return value;
        }
        break;
      }
      entry = Load32(base, entry + 8);
    }
  }
  return 0;
}

}  // namespace

// [[midasm_hook]] 0x822C36A0 (sub_822C36A0, the end of the main thread's frame, before its waits): once per frame.
void GHAerosmithQuitPromptHook() {
  const uint8_t* base = GuestBase();
  if (!base) {
    return;
  }
  const bool title = FindElement(base, kTitleContainer) != 0;
  const bool main_menu = FindElement(base, kMainMenuContainer) != 0;
  const bool popup = FindElement(base, kPopupWarning) != 0 || FindElement(base, kQuitWarning) != 0;
  static int last_state = -1;
  const int state = (title ? 1 : 0) | (main_menu ? 2 : 0) | (popup ? 4 : 0);
  if (state != last_state) {
    REXLOG_DEBUG("[quit_prompt] title {} main menu {} popup {}", title, main_menu, popup);
    last_state = state;
  }
  // The same frame's title screen: the controller pressing there gets the profile (src/players.cpp).
  ghaerosmith::SetTitleScreen(title && !popup);
  if ((title || main_menu) && !popup) {
    rex::ui::OfferQuitPrompt();
  }
}
