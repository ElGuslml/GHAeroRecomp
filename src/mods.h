// ghaerosmith - the launcher's Mods page (src/mods.cpp).

#pragma once

#include <cstdint>

namespace ghaerosmith::mods {

// The mods' script changes registered (src/script_globals.cpp), once, before the interpreter's hooks first run: from
// src/debug_menu.cpp.
void Configure();

// How much of the instrument's track stays on a miss, in percent (100 with ghaerosmith_miss_lower off; 0 = muted as on
// the console).
int32_t MissVolume();

}  // namespace ghaerosmith::mods
