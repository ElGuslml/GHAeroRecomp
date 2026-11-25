// ghaerosmith - the launcher's Mods page: the note highway's style and opacity, as GH5's and GH Warriors of Rock's (their
// src/mods.cpp). Each is a change of the game's own script data (src/script_globals.cpp), made when the game starts.
//
// - Highway style: highway_normal (scripts\guitar\guitar_tweaks.qb, [255 255 255 255]) is the highway's color, the
//   texture's multiplier: setup_highway creates the highway's sprite (Highway_2D<p>) with rgba = $highway_normal,
//   GuitarEvent_StarPowerOff sets it back, and a miss's highway_pulse_black morphs it to $highway_pulse ([0 0 0 255])
//   and back. Dark is the community's black highway: [0 0 0 255]. (This game's highway_starpower is read by no script.)
// - Highway opacity: the alpha of highway_normal and highway_pulse, and the cut-out of the venue under the highway left
//   out. create_highway_prepass adds a prepass viewport to bg_viewport with a highway-shaped object seen by its own
//   camera, and if $highwayvisible = ON calls enable_highway_prepass (the viewport's prepass active: the venue is not
//   drawn under the highway, black behind a see-through one). With the opacity below 100, create_highway_prepass reads
//   highwayvisible as OFF: the prepass is never turned on and the venue shows through. (GH5 cleared the prepass flag
//   of the window instead: its create_highway_prepass makes a window element.)

#include "mods.h"

#include <rex/cvar.h>
#include <rex/logging.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "script_globals.h"

REXCVAR_DEFINE_STRING(ghaerosmith_highway_style, "original", "GHAerosmith",
                      "The note highway's look: original, or dark (the community's black highway)");
REXCVAR_DEFINE_INT32(ghaerosmith_highway_opacity, 100, "GHAerosmith",
                     "The note highway's opacity in percent: below 100 the venue shows through it (the game's cut-out of "
                     "the venue under the highway left out); 100 = as on the console");

namespace ghaerosmith::mods {

void Configure() {
  static bool configured = false;
  if (configured) {
    return;
  }
  configured = true;
  std::string summary;

  const std::string style = REXCVAR_GET(ghaerosmith_highway_style);
  if (style == "dark") {
    for (int i = 0; i < 3; ++i) {
      script::Set("highway_normal[" + std::to_string(i) + "]", 0);
    }
    summary += " highway style dark";
  } else if (style != "original" && !style.empty()) {
    REXLOG_WARN("[mods] ghaerosmith_highway_style \"{}\": not a style (original, dark); the original", style);
  }

  const int32_t opacity = std::clamp(REXCVAR_GET(ghaerosmith_highway_opacity), 0, 100);
  if (opacity < 100) {
    const double alpha = std::round(255.0 * opacity / 100.0);
    script::Set("highway_normal[3]", alpha);
    script::Set("highway_pulse[3]", alpha);
    script::ReadAsName("create_highway_prepass", "highwayvisible", "off");
    summary += " highway opacity " + std::to_string(opacity) + " %";
  }

  if (!summary.empty()) {
    REXLOG_INFO("[mods]{}", summary);
  }
}

}  // namespace ghaerosmith::mods
