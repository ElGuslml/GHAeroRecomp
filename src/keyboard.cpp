// ghaerosmith - the keyboard's song mark: where a song runs.
//
// The SDK asks the game's tap gate (rex::input::mnk::SetTapGate) whether a song runs. With keyboard_guitar_layout =
// "gh3" (src/startup.cpp; SDK change 126) the keyboard plays as a guitar in the menus (green selects, red goes back,
// the arrows move) and, while the gate is open, gives the game its fret keys on Guitar Hero III's controller layout
// (LT, LB, RB, RT, A), where this game's controller code reads them. With mnk_tap_target = "dpad_down" (off here) the
// keyboard's pad would also strum on each fret key pressed while it is open. A player's guitar logic in a song (the
// script function GuitarInputLogicPerFrame, sub_821235E0, every frame for each player from script 0x057C9B84's loop)
// marks the time; the gate is open while that mark is recent. As in GH Metallica (GHMetalRecomp's src/keyboard.cpp).

#include "keyboard.h"

#include <rex/input/mnk/mnk_input_driver.h>
#include <rex/ppc/context.h>

#include <atomic>
#include <chrono>
#include <cstdint>

namespace {

using Clock = std::chrono::steady_clock;

// When a song last updated its players (steady clock ticks), 0 if never.
std::atomic<int64_t> g_song_frame{0};

// A song's frames come at least 30 times a second; a quarter of a second without one is the menus (or the pause
// menu, where the players are not updated).
bool InSong() {
  int64_t last = g_song_frame.load(std::memory_order_relaxed);
  return last != 0 && Clock::now().time_since_epoch().count() - last <
                          std::chrono::duration_cast<Clock::duration>(std::chrono::milliseconds(250)).count();
}

}  // namespace

// [[midasm_hook]] 0x821235E0: the entry of sub_821235E0, GuitarInputLogicPerFrame, a player's guitar in a song.
void GHAerosmithSongFrameHook() {
  g_song_frame.store(Clock::now().time_since_epoch().count(), std::memory_order_relaxed);
}

namespace ghaerosmith {

void InitializeKeyboard() { rex::input::mnk::SetTapGate(InSong); }

bool PlayingSong() { return InSong(); }

}  // namespace ghaerosmith
