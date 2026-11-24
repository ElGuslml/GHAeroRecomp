# The fixes ported from GH Metallica

**Status: ported, waiting for Gustavo's play test (2026-10-27).** Guitar Hero: Aerosmith runs on Guitar Hero III's
engine (the executable is `gh3_xp1_Final.exe`; GH Metallica's, `gh4_1_burn_Final.exe`, is the next engine, Guitar
Hero World Tour's), built with XDK 2.0.6683. Its first boot had several of GH Metallica's problems. GHMetalRecomp's
fixes were ported one by one: a function matcher over both executables' instruction indexes found only a few of their
functions (about 1900 of 35000: another engine and another compiler, 60000 functions here), so each site was found
again by its role (the script functions' table, the scripts in `DATA/COMPRESSED/PAK/qb.pab.xen`, stack samples,
temporary probes). Each section says what the fix does here and how to turn it off; the investigations are in
GHMetalRecomp's `docs/` and `CLAUDE.md`.

Checked here: the menus and a Quickplay song at 60 and 120 FPS, the CPU, the quit prompt, the language, NativeXRenderer
(the observer at 100 %).

## The guitar: "An unsupported guitar peripheral has been detected"

**Symptom:** with a Guitar Hero guitar connected (an X-plorer), the game stopped before its title screen asking for a
Guitar Hero guitar or a controller. **Cause:** SDL reports every guitar with one type, which the SDK turned into
XInput's GUITAR subtype (Rock Band's guitars); Guitar Hero's guitars are GUITAR_ALTERNATE on the console, and this
engine takes only those as guitars (`IsGuitarController`, `sub_82107D40`) and calls the others unsupported
(`IsStandardGuitarControllerPluggedIn`, `sub_8210AA28`). GH Metallica takes both. **Fix:** the SDK's `guitar_subtype`
(its change 51) set to 7 in `ghaerosmith.toml`, and `gamepad_subtype = 7`: a controller or the keyboard plays as a
Guitar Hero guitar. **Off:** `guitar_subtype = 0`.

## The threads' waits (CPU)

**Symptom:** 2.4 cores in the main menu, two threads at 100 %. **Cause:** two loops wait by checking a condition again
and again, which costs nothing on the console: the main thread at the end of each frame (the vertical blank, then the
render thread; `sub_822C36A0`) and the render thread presenting a frame (two vertical blank counters; `sub_822CE808`).
The D3D library's waits for the GPU (`sub_824DCC98`) spin too at higher frame rates. **Fix:** `src/thread_waits.cpp`,
five hooks: a wait spins briefly, then sleeps; the vertical blank waits sleep until shortly before the vertical blank.
Measured: the menu 0.51 cores, a song 0.78 cores at 60 FPS. **Off:** `ghaerosmith_wait_smart = false` (slices only),
`ghaerosmith_wait_sleep_us = 0` (all off).

The generated code also keeps the registers in local variables (the codegen's `*_as_local` options, as GH Metallica:
setjmp `sub_82795FD0` and longjmp `sub_82795B80`, used only by the image decoders, whose six functions that call
setjmp share their registers): a song 0.91 → 0.78 cores.

## The song's clock

The engine's timer replaces the 50,000,000 Hz the system reports by the original console's real timebase rate,
49,875,000 Hz (`sub_82175EE0`); the SDK's timebase runs at 50 MHz, so the game's clock would run 0.25 % fast and the
notes drift ahead of the audio (GH Metallica: about 150 ms a minute). **Fix:** `src/timer.cpp`, a hook keeps the
reported frequency. **Off:** `ghaerosmith_timer_fix = false`.

## The audio

The game uses FMOD Ex, as GH Metallica: `ghaerosmith.toml` sets `audio_callback_min_interval_us = 3000` (the audio
output called at the console's pace; in GH Metallica the menu music was noise without it) and `audio_maxqframes = 4`
(the SDK's shortest output queue). The SDK's XMA decoder carries each packet's skip count into the next buffer (for
songs of several interleaved streams).

## The crowd's claps on star power

The crowd claps on the beat while star power is on, but late by the player's audio and video calibration (also on the
console): the claps come from the fretbar handler (script `0x5E4AD8C0`), which waits 0.18 s, and the fretbar events
are registered late by the calibration. Here the calibration is not kept in a script global: the song's setup passes
the handler's offset to the fretbar iterator, which stores it. **Fix:** `src/crowd.cpp`, a hook records the offset at
the iterator's setup (`sub_82116B20`) and one in the script function Wait (`sub_8225FD10`) takes the calibration off
the handler's wait. **Off:** `ghaerosmith_crowd_claps_fix = false`.

## Quitting from the title screen and the main menu

B on the title screen ("JUST PUSH ANY BUTTON TO PLAY...") or the main menu asks whether to quit (A quits, B cancels):
`src/quit_prompt.cpp`, a hook in the main thread's frame looks up the title screen's container (`0x9884EF17`) and
`main_menu_text_container` in the screen elements' table (`0x82A48430`), and not while a warning popup is shown.
**Off:** `quit_prompt = false`.

## A second controller

As in GH Metallica: the controller that presses a button at the title screen plays with the profile, the others are
its guests (`signin_guests`, `src/players.cpp`, the title screen found by the quit prompt's hook);
`signin_ui_cancel = true` answers a sign-in request as a cancel. **Off:** `ghaerosmith_profile_follows_press = false`.

## The keyboard

The keyboard as a guitar (`mnk_mode`): frets D F J K L, Star Power Space; GH Aerosmith Launcher's Keyboard page changes
them. A song is told apart by a hook at `GuitarInputLogicPerFrame` (`sub_821235E0`, `src/keyboard.cpp`). Since
2026-11-24 the keyboard plays a song on the game's own controller layout (no strum) and keeps a guitar's keys in the
menus: [The keyboard](keyboard.md).

## Frame rates above 60

**Experimental.** `ghaerosmith_frame_rate` (60, 120, 144, 160, 165, 240; the launcher's frame rate row) sets the
refresh rate the game paces itself by; its clock is real time (the frame's length is measured, `sub_82176050`), so the
songs keep their speed. What counts frames is corrected by `src/framerate.cpp` (`ghaerosmith_frame_rate_fix`):
- The scripts' `Wait N gameframes` (249 scripts) last the console's N / 60 s, as in GH Metallica, but this engine runs
  much of its per-frame logic from scripts that wait one game frame in a loop (the guitar's input, star power's
  activation and drain, the note holds, the score, the iterators, the movies, the menus that poll the controllers):
  those 67 keep running every frame (held, they lost presses: the intro's movies could not be skipped at 120). Held
  although they poll the controllers, because they step by the pass: the lag calibration (its circles move 1/60 s a
  pass and the beat sounds when one reaches the line: Gustavo heard and saw both calibrations run fast) and star power
  by tilting the guitar (30 passes tilted).
- A held script that asks for the frame's time (`GetDeltaTime`) is given the time since its previous pass: the highway
  rises at a song's start with a speed damped each pass (`move_highway_2d`), and stayed low at 120 FPS and below the
  screen at 160 (Gustavo).
- star_power_whammy, a script here (C++ in GH Metallica): the bar's movement and its 5-pass countdowns as at 60, and
  star power grown by the time since its previous pass.
- The button presses the held scripts poll (ControllerMake) are given to them from the frames they skip.
- The loading screen's wheel (the boot's) turns 0.175 radians a frame, and its wave mode moves by a step a frame: each
  step scaled by 60 times the time since its previous one (GH Metallica's star).
- These count by the frames' real length (2026-11-24, GH5's): the margins that pick the frame a script wait ends in and
  the window of presses given to held scripts use the main thread's frames as measured at their end (`sub_822C36A0`,
  hook `0x822C3760`: an average of about ten frames), not 1 / the chosen rate, so they keep the console's speed when
  the game runs below its rate. Checked with a temporary probe: 6.252 ms at 160, 4.167 ms at 240.
- The hair, the clothes and the guitar cables (Havok physics; not in GH Metallica): the world is stepped by a fixed
  1/60 s each frame (`sub_82250750`), while the bodies that follow the animated bones are given a velocity to reach their
  targets in the frame's time (`applyHardKeyFrame`, `sub_82697090`). At 160 FPS each step carried them 2.67 times as
  far, past the target and back, growing: the hair and the cables stretched across the stage in spikes (Gustavo, at 120
  and 160 on both renderers; steady below about 100). Above 60 Hz the world is stepped by the frame's time.
**Off:** 60 in the launcher; `ghaerosmith_frame_rate_fix = false` (to compare).

## Ultrawide

As GH Metallica's (`src/widescreen.cpp`, 8 hooks, found again in this engine): every 16:9 perspective is widened by
the screen's factor when the engine builds it (`sub_8222F590`, Hor+), and the 2D layer (sprites, text, the highway's
board, the sustains' tails, the movies, the clip windows) is narrowed to 16:9 in the center; outside a song (the
highway's board not drawn, or a movie or the loading screen up) black bars cover the sides. GH Metallica's hook for
the 2D particles has no counterpart: here they are screen elements, drawn as sprites. With NativeXRenderer the image is
rendered at the screen's aspect ratio. Checked at 21:9 on both renderers. **Off:** 16:9 in the launcher
(`ghaerosmith_ultrawide = false`); `ghaerosmith_ultrawide_hud = false` stretches the 2D layer;
`ghaerosmith_menu_bars = false` shows the picture behind the menus at their sides.

## Language

The game has no language menu: it picks its text pack from `XGetLanguage` (French, German, Italian, Spanish, English
otherwise; checked: Spanish). The launcher's language row sets it.

## The move of 2026-11-13: towards GH Metallica 1.0.0-beta.1

The SDK (`4348a34`, the `ghmetal` branch: the audio and graphics card settings, the precise gate, the cursor, online
play off), NativeXRenderer (`9de7f7e`: D136-D144) and RecompLauncher 3.21.1 (the 3.18 redesign, Settings > Audio, the
graphics card, Settings > Game's switches). Built, check_switch 849 / 0 / 0; booted on a hidden desktop with both renderers to the title screen (the four videos skipped, no error). Not played.

- **Skipping the videos at the start** (Settings > Game > "Skip the intro videos", `ghaerosmith_skip_intro`, off): the
  boot (`bootup_sequence`) waits for the legal notice (until 6 s have passed since the loading began: the loading
  covers it here, about 7 s) and plays `atvi`, `ro_logo`, `ns_logo` and `intro`. `src/intro.cpp` makes the script
  function PlayMovie (hook 0x8227C548 in `sub_8227C4F0`, the same code as GH Metallica's) find none of the four the
  first time each is asked for: the title screen as soon as the game has loaded. No legal Wait to shorten here (GH
  Metallica's hook on Wait): `start_legal_timer` is called by no script.
- **The red border at ultrawide** (`ghaerosmith_ultrawide_overlays`): in a song a 2D overlay over the whole screen
  covers the whole width (the sprite hook leaves its pieces as they are, GH5's rule). Not seen in this game.
- **The SDK's settings** (`src/startup.cpp`): V-Sync (`native_x_renderer_present_mode = "vsync"`,
  `vblank_follows_display`, `vblank_timer_spin_us`), low latency audio's buffer and queue, the audio threads' priority
  and `audio_keep_time` (the music stays with the notes after the audio thread is held up), a controller as a guitar
  (`gamepad_whammy_rest`, `gamepad_tap_sources`: the launcher's strum switch on a controller's page), `debug_hotkeys =
  false`, the cursor hidden after 2 s.
- **V-Sync off on a G-Sync / FreeSync display** (`src/options.cpp`, GH Metallica's): the game's vertical blank just under
  the display's exact rate when the frame rate chosen is that rate.

### The render thread woken by the counters (V-Sync with G-Sync)

GH Metallica's of 2026-11-04 (its `src/thread_waits.cpp`). The render thread's two waits (the swap counter 0x82AE38E0,
the vertical blank counter 0x82AE38D8) and the main thread's frame end (the vertical blank counter) slept until shortly
before the advance they expected, then in slices. Now they block on an event the game's callbacks set right after they
advance the counters (hooks 0x82175C10 in `sub_82175BF0`, the swap's, and 0x82175BC8 in `sub_82175B98`, the vertical
blank's; an event per wait, the vertical blank's set for both threads): the thread wakes some tens of microseconds after
the counter. With the SDK's timer (`vblank_timer_spin_us`) this is what took the saw tooth out of GH Metallica's frame
time on Gustavo's G-Sync displays. Checked on a hidden desktop at 120 FPS, V-Sync off: the callbacks signal 120 times a
second; the title screen's presents (PresentMon, 25 s) the same within noise as before (92.5-97.2 % within 0.5 ms of the
period with the events, 93.1 % without; 91.5 % with the slices alone): not a G-Sync display. **Off:**
`ghaerosmith_wait_smart = false` (slices only).

### A controller plays on the game's own layout (2026-11-13, Gustavo's choice)

Guitar Hero III plays a plain Xbox 360 controller (GH Metallica gives one the vocals, so its recompilation reports a
controller as a guitar; this one did too until now: `gamepad_subtype = 7`). Reported as a controller
(`gamepad_subtype = 0`), it plays on the game's layout, read from its code and tried with a virtual controller in a song:
the frets on **LT** (green), **LB** (red), **RB** (yellow), **RT** (blue) and **A** (orange), and **no strum**: the note
plays when a fret is pressed (`sub_82108000`: for a controller the strum is a change of the frets held; for a guitar the
strum bar); **Star Power** on the D-pad, any direction, or Back (`check_for_star_power_CFunc`, sub_821188D8); the
**whammy bar** on either stick, pushed from the centre (`control_whammy_pitchshift`: a controller's by the sticks'
length, a guitar's from its resting position, so the bar no longer reads pressed at rest). Real guitars stay guitars
(`guitar_subtype = 7`). The keyboard is a controller too: frets D, F, J, K, L; the arrows the D-pad (the menus, Star
Power in a song); Space Star Power; Enter or L select, Backspace back; X or P Start; Right Shift the whammy bar. The
launcher (3.21.2, `kGuitarControlsApart`) names a controller's and the keyboard's controls on this layout and a
guitar's on its frets; the keyboard's strum switch is gone (nothing to strum). `src/startup.cpp` rewrites the old
values once in an existing `ghaerosmith.toml` (`kChangedDefaults`, GH Van Halen's code); keys saved by the launcher
stay until the Keyboard page's Reset. **Back to the guitar's layout:** `gamepad_subtype = 7`.

Left for later: `docs/pending.md`.
