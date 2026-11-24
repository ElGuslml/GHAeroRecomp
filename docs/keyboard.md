# The keyboard: a guitar in the menus, the game's own layout in a song

**Symptom** (2026-11-24, Gustavo). With the keyboard, D (the green fret) did nothing in the menus or at "Press any
button": the menus only took L or Enter.

**Cause.** Since 2026-11-13 a controller plays on Guitar Hero III's own controller layout (the frets on LT, LB, RB, RT
and A; `gamepad_subtype = 0`), and the keyboard was set up as such a controller: D was LT. In a song LT is the green
fret, but the menus select with A and do not take LT.

**Fix.** `keyboard_guitar_layout = "gh3"` (rexglue-sdk change 126) with the keyboard on a guitar's keys
(`src/startup.cpp`, as in GH5 and GH Metallica): D, F, J, K and L are a guitar's green, red, yellow, blue and orange
(A, B, Y, X, LB), so in the menus D selects, F goes back and the arrows move. While a song runs (`src/keyboard.cpp`'s
song mark), the SDK gives the game those keys on LT, LB, RB, RT and A, where its controller code reads the frets: a key
pressed plays its note, no strum. Star Power on Space or Backspace, Start on Enter or X, the whammy bar on the right
arrow. A file with the keys of 2026-11-13 is moved to these once (`kChangedDefaults`); the launcher's Keyboard page
names the keys as the guitar's (RecompLauncher 3.33.2).

**Turn it off:** `keyboard_guitar_layout = ""` and the keyboard's keys set as a controller's in the launcher.
