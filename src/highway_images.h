// ghaerosmith - the note highway's picture from the player's folder (src/highway_images.cpp).

#pragma once

namespace ghaerosmith::highway {

// The picture chosen in the launcher (or a random one), asked of NativeXRenderer once at the start.
void Configure();

// Whether a random picture changes at each song (src/debug_menu.cpp then watches setup_highway).
bool Random();

// A highway made (setup_highway evaluating an expression): the first after 3 s without one is a new song.
void OnHighwaySetup();

}  // namespace ghaerosmith::highway
