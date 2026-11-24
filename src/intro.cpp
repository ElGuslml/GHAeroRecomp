// ghaerosmith - the boot's videos skipped: the game starts at its title screen.
//
// The boot is this engine's (Guitar Hero III's) script bootup_sequence (scripts\guitar\menu\bootup_menu_flow.qb):
// wait_for_legal_timer loops a game frame at a time until the global legal_timer is 1, which guitar_startup sets once it
// has loaded and 6 s have passed since its loading began (the legal notice is the loading screen: a load that takes 6 s
// leaves no wait; no Wait to shorten there, start_legal_timer's is called by no script); then four videos, each with
// PlayMovieAndWait: 'atvi' (Activision), 'ro_logo' (RedOctane), 'ns_logo' (Neversoft), 'intro' (the game's intro),
// then the title screen (read from the decoded scripts, GHMetalRecomp's tools/qb). PlayMovieAndWait (guitar_movies.qb)
// starts the video with the script function PlayMovie (sub_8227C4F0, the same code as GH Metallica's) and waits while
// IsMoviePlaying, as when a button skips a video. So PlayMovie returns false for those four, the first time each (a
// later request plays: the bonus videos): the boot goes on at once. GH Metallica's fix (its src/intro.cpp).

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

REXCVAR_DEFINE_BOOL(ghaerosmith_skip_intro, false, "GHAerosmith",
                    "Skip the videos at the game's start (Activision, RedOctane, Neversoft, the intro): the game starts "
                    "at its title screen once it has loaded; false = as on the console");

namespace {

constexpr std::array<std::string_view, 4> kBootMovies = {"atvi", "ro_logo", "ns_logo", "intro"};
std::array<std::atomic<bool>, kBootMovies.size()> g_skipped{};

bool Enabled() {
  static const bool enabled = REXCVAR_GET(ghaerosmith_skip_intro);
  return enabled;
}

const uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

// A string of guest memory, lowercase, at most 64 characters.
std::string GuestString(const uint8_t* base, uint32_t address) {
  std::string text;
  for (uint32_t i = 0; address && i < 64; ++i) {
    char c = char(base[address + i]);
    if (!c) {
      break;
    }
    text += char(std::tolower(static_cast<unsigned char>(c)));
  }
  return text;
}

}  // namespace

// [[midasm_hook]] 0x8227C548 (PlayMovie, sub_8227C4F0, after it read its string parameter `movie`, QBKey 0xE2A10D90,
// into 84(r1)): r3 = whether it was found. A boot video the first time it is asked for: not found, so PlayMovie
// returns false and plays nothing.
void GHAerosmithPlayMovieHook(PPCRegister& r1, PPCRegister& r3) {
  if (!Enabled() || !(r3.u32 & 0xFF)) {
    return;
  }
  const uint8_t* base = GuestBase();
  if (!base) {
    return;
  }
  const std::string movie = GuestString(base, Load32(base, r1.u32 + 84));
  for (size_t i = 0; i < kBootMovies.size(); ++i) {
    if (movie == kBootMovies[i] && !g_skipped[i].exchange(true)) {
      REXLOG_INFO("[intro] boot video '{}' skipped", movie);
      r3.u64 = 0;
      return;
    }
  }
}
