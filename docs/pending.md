# Pending

What GH Metallica's recompilation has that this one does not yet, and what is left before a release (2026-11-13,
after the move towards GH Metallica 1.0.0-beta.1, `docs/fixes-from-gh-metallica.md`). Each item says why it was left:
this game runs on Guitar Hero III's engine, so each of GH Metallica's fixes has to be found again in other code.

- **The audio's pacing.** GH Metallica and GH Van Halen call the game's audio callback as soon as the device frees a
  frame (`audio_callback_min_interval_us = 0`): spaced 3 ms apart, a slower PC crackled (GHMetalRecomp's
  `docs/audio-crackling.md`). That needed FMOD's mixing moved to the SDK's audio worker first (their `src/fmod_mix.cpp`):
  this game's FMOD (Ex 4.06) polls its ring from its own thread, and without the spacing its music is noise. So 3000
  stays here until `fmod_mix.cpp` is ported to this FMOD.
- **The game's debug menu.** `enable_button_cheats` is 0 in this game's scripts and `create_main_menu` /
  `create_pause_menu` read it, as GH Smash Hits'; but Guitar Hero III's script interpreter is not GH Metallica's (no
  function resolves a `$name` component with its code), so its expression evaluation and its globals' resolve have to
  be found first.
- **The screen effects' switches** (GH Metallica's `src/screen_fx.cpp`): this engine's post-processing composite is not
  mapped.
- **V-Sync with G-Sync, the game's side:** done on 2026-11-13 (`docs/fixes-from-gh-metallica.md`, "The render thread
  woken by the counters"); left: a look on a G-Sync / FreeSync display, where GH Metallica's saw tooth showed.
- **Each controller's kind** (GH Metallica's issue 4: a port's kind asked again outside a song): this engine tells
  guitars apart with other code (`IsGuitarController`, sub_82107D40).
- **To check with NativeXRenderer:** the bloom's lattice (D127, `unscaled_screen_fractions`, at 1440p and above),
  alpha to coverage at 1x (D138: hair and cut-out edges against the Xenos path), and the shadow quality row
  (`kShadowQuality`, off).
- **Online play:** this engine's online code not studied yet.
- **Release:** the players' README (`res/README.txt` is the template's), README.md, the launcher's look.
- **Not applicable:** drums and singing (this game has guitar and bass only), the tap notes' trail (no tap notes), GH
  Metallica's band strings script (not in this game), its job queue's lock (no such queue).
