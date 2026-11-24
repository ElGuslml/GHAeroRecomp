// ghaerosmith - the profile follows the controller that starts the game.
//
// On the console the controller that presses a button at the title screen ("JUST PUSH ANY BUTTON TO PLAY...") plays
// with its own profile. Here there is one profile, the first controller's (the order Windows connected them in); in
// GH Metallica another controller that started the game got the game's "not signed in" message. While the title screen
// is shown (src/quit_prompt.cpp finds its container each frame), the SDK's primary press window is open: the
// controller that presses first becomes user 0, the profile's, and the other controllers are its guests ("Name (1)",
// the SDK's signin_guests, set in src/startup.cpp), as when guests sign in on the console. GH Metallica's
// docs/second-controller.md.

#include "players.h"

#include <rex/cvar.h>
#include <rex/input/input_system.h>
#include <rex/logging.h>

REXCVAR_DEFINE_BOOL(ghaerosmith_profile_follows_press, true, "GHAerosmith",
                    "The controller that presses a button at the title screen plays with the profile; off: the "
                    "first controller connected does, as before");

namespace ghaerosmith {

void SetTitleScreen(bool shown) {
  static bool open = false;
  const bool want = shown && REXCVAR_GET(ghaerosmith_profile_follows_press);
  if (want == open) {
    return;
  }
  open = want;
  rex::input::SetPrimaryPressWindow(open);
  REXLOG_DEBUG("[players] the title screen's press window {}", open ? "open" : "closed");
}

}  // namespace ghaerosmith
