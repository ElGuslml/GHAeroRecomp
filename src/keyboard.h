// ghaerosmith - the keyboard's strum on each fret, only in a song (src/keyboard.cpp).

#pragma once

namespace ghaerosmith {

// Before the guest starts: the SDK's keyboard strums (mnk_tap_target) only while a song runs.
void InitializeKeyboard();
// Whether a song is playing now: a player's guitar logic ran within the last quarter of a second (the gate above). Off
// in the pause menu, where the players are not updated. For NativeXRenderer's summary of the song's frames and the
// song's frame blanks (src/framerate.cpp).
bool PlayingSong();

}  // namespace ghaerosmith
