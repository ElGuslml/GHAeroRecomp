// ghaerosmith - the game's script globals changed in guest memory, and globals read as another value by one script: the
// mechanism the mods share (src/mods.cpp; GH5's src/script_globals.h, made in GH Warriors of Rock).

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ghaerosmith::script {

// A name's QBKey: the CRC-32 of its lower-case letters without the final inversion.
uint32_t QbKey(std::string_view name);

// A global's value changed at a path, before the game reads it: "global", ".field", "[index]" (an array's element or a
// vector's component), "[*]" (every element) and ".*" (every field that is a struct), e.g. "highway_normal[3]". An
// integer stays an integer (rounded). Applied at the first "$" read of any global (every global is loaded at the start
// and kept), so the change reaches the scripts the interpreter runs and the compiled ones alike.
void Set(std::string path, double value);
void Scale(std::string path, double factor);

// A global read as another value by the script named `script` only ("$name" in that script); the global keeps its
// value. An integer, or a name (a checksum, as ON and OFF). Only before the interpreter's hooks first run (src/mods.cpp's
// Configure): the reads are looked up without a lock.
void ReadAs(std::string_view script, std::string_view global, int32_t value);
void ReadAsName(std::string_view script, std::string_view global, std::string_view name);

// Whether changes wait for their first read, and whether the interpreter's hooks must know each read's script (a read as
// another value; src/debug_menu.cpp).
bool Pending();
bool NeedsScriptNames();

// From src/debug_menu.cpp's GHAerosmithGlobalResolveHook: the component (r3) is about to become `global`'s value, read
// by the script named `script` (0 when not known). True when the component was written here (read as another value).
bool OnResolve(uint8_t* base, uint32_t component, uint32_t global, uint32_t script);

}  // namespace ghaerosmith::script
