# Mods

**Status (2026-11-24):** the launcher's Mods → Highway: the note highway's **scale**, **length** and **perspective**
(Shape) and its **style** and **opacity** (Look); Mods → Sound: the **whammy bar's sound** and the **instrument's volume
on a miss**; as in GH5 and GH Warriors of Rock. All are as on the console by default. The highway's are changes of the
game's own script data when it starts (`src/mods.cpp`, `src/script_globals.cpp`), the sound's hooks on two of its C
functions.

- **Highway scale** (`ghaerosmith_highway_scale`, 80-120 %, one player): the highway, its gems, sidebars and strikers
  smaller or larger, anchored at the strike line. Unlike GH5 the rock meter and the Star Power meter stay where they are
  (this game has no table that scales them with the highway): at 120 % the orange striker comes close to the rock meter.
- **Highway length** (`ghaerosmith_highway_length`, 100, 125, 150, 170 %): each note shown that much earlier, the
  console's highway continued beyond its far end in its own perspective; the notes keep their speed at the strike line.
  Limited so that the far end stays below y = 50 of 720.
- **Highway perspective** (`ghaerosmith_highway_perspective`, 25-150 %): flatter below 100 %, steeper above. The notes
  take the same time to cross, so they reach the strike line slower below 100 % (0.78 at 75 %) and faster above;
  **Notes at the console's speed** (`ghaerosmith_highway_keep_speed`) changes the times on the highway to make up for it.

Guitar Hero III draws its highway as GH5 does, with the same geometry (350 high, 160 wide at the top, 512 at the
bottom, the strike line at 655, 1024 rows), but builds the rows in a script (`generate_pos_table`) from a perspective
table indexed by the highway's height (`heightPerspFactTable`, `heightPerspExpTable`): the mods write the new
projection into the entry of the new height, for one player and for two (270 high, 170 wide).

- **Highway style** (`ghaerosmith_highway_style`: `original`, `dark`): Dark is the community's black highway. The
  highway's color (`highway_normal`, the texture's multiplier) becomes black, so the gems, the sustains and the lines
  stand out against it.
- **Highway opacity** (`ghaerosmith_highway_opacity`: 100 to 30 %): below 100 the venue shows through the highway. The
  alpha of the highway's color and of a miss's flash (`highway_pulse`), and the game's cut-out of the venue under the
  highway left out: `create_highway_prepass` reads `highwayvisible` as OFF, so the prepass that keeps the venue from
  being drawn there (black behind a see-through highway) is never turned on.

- **Whammy sound effect** (`ghaerosmith_whammy_sound`, on): off, the whammy bar no longer bends a held sustain's pitch.
  The bar still wobbles the note on the highway and fills Star Power. Guitar Hero III bends the guitar's sound bus in a
  script (`set_whammy_pitchshift`: `setsoundbusseffects` with a PitchShift of `1 - bar x 0.057`); with the switch off,
  `SetSoundBussEffects` does nothing when that script calls it. For every player, a remote online one's too.
- **Instrument volume on a miss** (`ghaerosmith_miss_lower`, on, and `ghaerosmith_miss_volume`, 0-100 %, 0): how much
  of your guitar's track stays after a missed note or a strum without one, until the next hit (0 % = muted as on the
  console; the switch off = 100 %). The miss scripts set `player_status.guitar_volume` to 0 and call
  `UpdateGuitarVolume`, which sets the track's volume to that percentage; that call reads the chosen percentage instead
  of 0. The game's other mutes stay (a sustain let go early, the start before the first note), as do two players on
  the same part (they share a track) and the game's value of `guitar_volume` (the scripts and the network messages read
  it).

## How

Guitar Hero III's interpreter keeps the script globals in a table (`0x82B94030`). The changes are written into it at the
first `$name` read of any global (`sub_8219BF30`, hooked by `src/debug_menu.cpp`), at the boot, before a menu or a song
reads them; a read as another value in one script uses the same hook and the script being evaluated. The log says what
was changed (`[mods]`, `[script globals]` lines).

Checked in Dream Police (one player): Dark at 50 %, the highway black and the stage seen through it; scale 85 % and
115 %; length 150 % with perspective 75 %; length 170 % with perspective 150 %; perspective 25 % with the console's
speed. Two players' highways not seen yet. The miss at 50 % (the log's `a miss ... kept at 50 %`, the next misses
staying there). The whammy switch was not heard yet (it acts only while a sustain is held); its hook was checked to see
the calling script's name the way it will look for `set_whammy_pitchshift`.

## Turning it off

100 %, Original and the switch off in the launcher (the defaults), or in `ghaerosmith.toml`:
`ghaerosmith_highway_scale`, `_length`, `_perspective` and `_opacity` at 100, `ghaerosmith_highway_style = "original"`,
`ghaerosmith_highway_keep_speed = false`, `ghaerosmith_whammy_sound = true`, `ghaerosmith_miss_lower = true`,
`ghaerosmith_miss_volume = 0`.
