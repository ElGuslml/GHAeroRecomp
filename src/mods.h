// ghaerosmith - the launcher's Mods page (src/mods.cpp).

#pragma once

namespace ghaerosmith::mods {

// The mods' script changes registered (src/script_globals.cpp), once, before the interpreter's hooks first run: from
// src/debug_menu.cpp.
void Configure();

}  // namespace ghaerosmith::mods
