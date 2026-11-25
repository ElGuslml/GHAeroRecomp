# Mods

**Status (2026-11-24):** the launcher's Mods → Highway → Look: the note highway's **style** and **opacity**, as in GH5
and GH Warriors of Rock. Both are off by default (the console's highway). Each is a change of the game's own script
data when it starts (`src/mods.cpp`, `src/script_globals.cpp`).

- **Highway style** (`ghaerosmith_highway_style`: `original`, `dark`): Dark is the community's black highway. The
  highway's color (`highway_normal`, the texture's multiplier) becomes black, so the gems, the sustains and the lines
  stand out against it.
- **Highway opacity** (`ghaerosmith_highway_opacity`: 100 to 30 %): below 100 the venue shows through the highway. The
  alpha of the highway's color and of a miss's flash (`highway_pulse`), and the game's cut-out of the venue under the
  highway left out: `create_highway_prepass` reads `highwayvisible` as OFF, so the prepass that keeps the venue from
  being drawn there (black behind a see-through highway) is never turned on.

## How

Guitar Hero III's interpreter keeps the script globals in a table (`0x82B94030`). The changes are written into it at the
first `$name` read of any global (`sub_8219BF30`, hooked by `src/debug_menu.cpp`), at the boot, before a menu or a song
reads them; a read as another value in one script uses the same hook and the script being evaluated. The log says what
was changed (`[mods]`, `[script globals]` lines).

Checked: Dream Police with Dark at 50 %: the highway black and the stage seen through it.

## Not yet

GH5's highway scale, length and perspective (this engine's highway is drawn as GH5's, with its own perspective
constants), the whammy bar's sound and the instrument's volume on a miss.

## Turning it off

Original and 100 % in the launcher (the defaults), or `ghaerosmith_highway_style = "original"` and
`ghaerosmith_highway_opacity = 100` in `ghaerosmith.toml`.
