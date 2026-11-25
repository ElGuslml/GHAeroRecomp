# Screen effects

**Status (2026-11-24):** added. The launcher's Settings → Graphics → Screen effects has two switches, both on by
default (as on the console):
- **Bloom** (`ghaerosmith_fx_bloom`): the glow around lights and bright surfaces.
- **Depth of field** (`ghaerosmith_fx_depth_of_field`): the background blurred in some camera shots and in the guitar
  selection.

GH Metallica's and GH5's switches change their engines' post-processing composite. Guitar Hero III's engine has none:
each effect is an object of its own type, made when a script asks for it (`ScreenFX_AddFXInstance`). A switch turned
off keeps its effect from being made (`src/screen_fx.cpp`: a hook where the viewport's effects manager adds one, which
reads the effect's type from the script's parameters).

GH Aerosmith's scripts use few effects: the venues' look (a neutral color correction and a `simple_bloom`), the
camera shots' depth of field (`dof`), the fade to black (a `vignette`), the flashes and the brightness option
(`bright_sat`). The color correction changes nothing in this game (all its values neutral), and the vignette is the
fade to black, so neither has a switch. There is no film grain.

Checked in Dream Police: with both off, the log says `[screen fx] a bloom effect not made` and `a depth of field effect
not made` at each venue's start, and the background behind the guitarist is sharp (with them on, it is blurred and
glows).

Turning them off: the switches in the launcher, or `ghaerosmith_fx_bloom = false` and
`ghaerosmith_fx_depth_of_field = false` in `ghaerosmith.toml`.
