GHAeroRecomp {VERSION} - Guitar Hero: Aerosmith (Xbox 360), for Windows
========================================================================

A static recompilation of the Xbox 360 game to native Windows. The game's code is already translated and
compiled into GHAerosmithRecomp.exe: you do not need to compile anything. You do need your own copy of the
game, because this package contains no game files.

This is the first release. Not affiliated with Microsoft, Activision or the game's developers.


REQUIREMENTS

- Windows 10 or 11, 64-bit.
- A GPU with up-to-date drivers that support Vulkan (NativeXRenderer, the default renderer) or
  Direct3D 12 (the Xenos renderer).
- A CPU with SSE4.2 (practically any 64-bit CPU from 2009 on).
- Microsoft Visual C++ Redistributable 2015-2022 (x64). Most PCs already have it. If Windows says that
  MSVCP140.dll or VCRUNTIME140.dll is missing, install it from:
  https://aka.ms/vs/17/release/vc_redist.x64.exe
- Your Guitar Hero: Aerosmith disc as an ISO file (tested with the disc sold in the USA; about 4.3 GB once
  installed).
- An Xbox 360 guitar. Any other controller works too, and so does the keyboard. An Xbox One guitar (such as
  the PDP Riffmaster for Xbox) needs Microsoft GameInput: the installer offers it, and so does the launcher's
  Controls page while it is missing (Windows asks for permission to install it).


INSTALLATION

1. Either run the installer (GHAeroRecomp-{VERSION}-setup.exe: it installs for your Windows user, without
   administrator rights), or extract this zip to any folder, for example C:\Games\GHAeroRecomp.

2. Run "GH Aerosmith Launcher.exe". The first time, it asks for your Xbox 360 ISO, checks that it is the
   right game and extracts its files next to the launcher, then asks you to create your profile (your saves,
   achievements and playtime, shared with the other ElGus Recomps launchers). You only do this once.

   You can also copy the game files yourself instead: put the contents of the disc into the "assets"
   folder, so that assets\default.xex exists (extract-xiso, https://github.com/XboxDev/extract-xiso/releases,
   gets them out of an ISO).

3. Choose your options and press PLAY. Next time you can start GHAerosmithRecomp.exe directly: it uses the
   options you saved.

4. Calibrate: in the game's options, run the lag calibration once, so that the notes match what you hear on
   your own speakers or headphones.

If your default.xex is not the version this build was made from (CRC32 529115C6), the game warns you: a
different version will most likely crash.


"WINDOWS PROTECTED YOUR PC" / UNKNOWN PUBLISHER

The executables are not code-signed, so Windows SmartScreen warns about an unknown publisher the first time
you run them. To avoid it with the zip, unblock it before you extract it: right-click it, Properties, tick
"Unblock" at the bottom of the General tab, OK, and then extract it. Otherwise click "More info" and then
"Run anyway" in the warning (once for each program).


GH AEROSMITH LAUNCHER

The launcher works with the mouse, the keyboard or a controller. Changes are saved automatically and apply
when you press PLAY.

- Play: starts the game, with your current settings, achievements and updates at a glance. The first time,
  and after an update of the game or of your graphics driver, it first shows "Compiling shaders" for a
  moment: it prepares the game's graphics for your graphics card, so that the game does not stutter the
  first time it shows something (with NativeXRenderer). Skip starts the game at once; it then compiles
  them as you play.
- Your profile (your picture at the top right): hours played, streaks, the week's activity, your games
  and your favorite achievements; edit your name, picture and color, or switch profiles.
- Achievements: the game's 49 achievements, unlocked or not. In the game, F7 shows the same list.
- Controls: your guitars, controllers and the keyboard.
  - A guitar: what each of its buttons does. A guitar's frets are A, B, Y, X and LB of an Xbox controller,
    strum is the D-pad, the whammy bar the right stick and Star Power Back. Whammy bar: how far the game
    reads the bar. If it reads pressed with the bar at rest, or does nothing (some third-party guitars),
    press Assign, push the bar all the way and hold it, then let it go; X clears the assignment.
  - A controller plays on Guitar Hero: Aerosmith's own controller layout: the frets on LT, LB, RB, RT and A,
    no strum (pressing a fret plays its note), Star Power on the D-pad or Back, and the whammy bar on
    either stick, pushed from the center. Each of its buttons can be changed.
  - Keyboard (experimental): turn on "Play with keyboard and mouse" to play with the keyboard as a guitar,
    and change any key. In the menus the frets choose (D, the green fret, selects; F, the red fret, goes
    back) and the arrows move; in a song a fret key plays its note (D F J K L), Star Power is on Space or
    Backspace, Start on Enter or X, and the whammy bar on the Right arrow. Shift, Ctrl and Alt can be keys
    of their own (press and release one alone): each side is a separate key. F1 to F12 can be assigned
    too: a key on a control only presses that control.
- Mods (changes to the game itself; each applies when the game starts):
  - Highway: your own pictures on the note highway (NativeXRenderer only). Folder: where they are (PNG, JPEG,
    BMP or TGA, any size, stretched over the highway; by default Highways in the game's user data folder);
    A opens it in Explorer or changes it. Every guitarist has a highway of his own in this game: your picture
    replaces all of them.
    Image: A opens the gallery of that folder: Original (the game's own), Random (a different picture at
    each song) and your pictures; X marks a favorite, Y gives a bright picture a darkening of its own, LB
    and RB show All, Favorites (Random among them) or Recent, A uses the picture. Random among: All or
    Favorites. Dim bright pictures (Off by default): darkens every picture toward black so that the notes
    stand out (Automatic: as much as each one is bright). The preview at the right draws the highway with
    the game's own textures and notes, read from your game files: an approximation of the game's look.
    Highway scale (100 % by default): the note highway smaller or larger when you play alone, 80 to 120 %,
    from the strike line, with its gems. Highway length (100 % by default): each note shown that much
    earlier (125, 150 or 170 %), as in Clone Hero: the highway continues beyond its far end in its own
    perspective, and the notes reach the strike line at the same speed; the timing to hit them does not
    change; every player gets it; if it does not fit the screen it is shortened.
    Highway perspective (100 % by default): 25 to 150 %, flatter (as seen from higher, the far notes
    larger) or steeper; below 100 % the notes move slower near the strike line, above it faster. Notes at
    the console's speed (off by default, under the perspective): they reach the strike line as fast as at
    100 % whatever the perspective, so below 100 % each note shows for less time.
    Highway style (Original by default): Dark is the community's black highway. Highway opacity (100 % by
    default): below it the venue shows through the highway (the notes and lines stay solid).
  - Sound: Whammy sound effect (on by default): off, the whammy bar no longer bends your sustained notes'
    pitch (it still wobbles them and fills Star Power). Lower the track on a miss (on by default, as on the
    console), and under it Instrument volume on a miss (0 %, muted, by default, as on the console): how much
    of your guitar's or bass's track stays when you miss a note or strum without one, 0 to 100 %; with two
    players on the same part it does not change.
  - Tools: Debug keys (off by default): F3, F4 and ` open the developer's windows in the game (frame times,
    settings, console).
  - Custom textures (NativeXRenderer only, no launcher row): an image in the "textures" folder next to
    GHAerosmithRecomp.exe, named by a texture's hash (16 hexadecimal digits, then anything; .png, .jpg or
    .jpeg; subfolders too), replaces that texture. Joe Perry's note highway is 3452F9C82296C39C (512x1024;
    it repeats along the highway, so its top and bottom should join), for example "3452F9C82296C39C my
    highway.png"; each guitarist has his own. To find others, add to ghaerosmith.toml
    native_x_renderer_texture_log = true (each texture's hash in the log) or native_x_renderer_texture_dump =
    true (each one as a DDS file in texture_dump). The Highway's pictures win over this folder for the
    highway.
- Settings:
  - Display: the monitor, fullscreen, borderless or window, the resolution, the image format (fill a screen
    wider than 16:9, where the stage is seen wider while the note highway, the HUD and the menus stay 16:9
    in the center, or keep 16:9 with bars). Smoothness: the frame rate, 60 FPS (original), and 72, 75, 90,
    100, 120, 144, 160, 165 or 240 FPS (experimental; the songs keep their speed and stay in sync at every
    frame rate), and 300, 360, 480 or Unlimited (experimental, with V-Sync off: with V-Sync on the game runs
    at your monitor's rate), and V-Sync (on by default: no tearing, and no extra delay; leave it on with a
    G-Sync or FreeSync monitor too).
  - Graphics: the renderer and the graphics card. NativeXRenderer (the default) draws the game natively with
    Vulkan; Xenos emulates the Xbox 360 GPU instead (DirectX 12 or Vulkan): choose it if something looks
    wrong with NativeXRenderer, and tell us where. Graphics card: on a PC with two (a laptop's integrated and
    dedicated ones), the one the game draws with. Upscaling: AMD FSR 1 or NVIDIA NIS, with its quality and
    the internal resolution the game is drawn at. Image: antialiasing, sharpness and anisotropic filtering.
  - Effects: the game's screen effects, the bloom and the depth of field, each on by default.
  - Audio: the output device (Windows default, or another one), the channels, the volume (0 to 200 %: above
    100 % the game is louder than its own mix and its loudest moments can distort), mute while another
    window is in front, and Low latency audio (off by default): the game takes your audio device in
    exclusive mode and the sound comes sooner; other programs cannot play on that device while the game
    runs. Calibrate the lag again after changing it.
  - Game: the launcher's language and the game's (English, French, German, Italian or Spanish; the game
    has no language menu of its own). Skip the intro videos (off by default): the videos at the start are
    skipped, so the main menu comes sooner. Game debug menu (off by default): the debug menu its developers
    used, in the main menu and the pause menu (its texts in English; Unlock All changes your save).
  - Updates: the launcher checks for a new version when it starts (you can turn that off) and installs it
    for you, keeping your saves, settings and game files. Get beta versions (off by default) offers the betas
    too. Play shows a new version as a button when there is one.


MORE THAN ONE CONTROLLER

The first controller to press a button on the title screen plays with your profile and its saves. The other
controllers join as its guests, as on the Xbox 360 ("Name (1)", "Name (2)"): they play without saves of their
own.


QUITTING THE GAME

On the title screen or in the main menu, press B (the red fret): the game asks whether to quit. A (the green
fret, or Enter) quits, B (or Escape) goes back.


STEAM BIG PICTURE

Add GHAerosmithRecomp.exe as a non-Steam game: it starts straight into the game with the options saved by
GH Aerosmith Launcher. You can also add "GH Aerosmith Launcher.exe" to change options with a controller.


FILES

- ghaerosmith.toml          Base settings. Created with the defaults this game needs if it is missing.
- ghaerosmith_options.toml  What GH Aerosmith Launcher saves. Loaded after ghaerosmith.toml.
- logs\                     Log files: the game's (ghaerosmith_NNN.log) and the launcher's (launcher_NNN.log,
                            with a .dmp file when it stopped responding). Attach the latest ones when you report
                            a problem.
- userdata\                 The shader cache and your highway pictures (Highways). Keep this folder if you move
                            the game to another folder. If the game folder cannot be written (for example under
                            Program Files), it goes to Documents\ghaerosmith instead.
- Your saves, achievements and playtime are in your profile: %APPDATA%\ElGuslml Recomps.

Command line options (--name=value, for example in Steam launch options) win over both files, e.g.
--user_language=1 (English), --ghaerosmith_frame_rate=120, --native_x_renderer=false (the Xenos renderer) or
--gpu_adapter=10DE:2782 (a graphics card by its PCI IDs, vendor:device in hexadecimal, as the launcher's
Settings > Graphics writes it).


REPORTING A PROBLEM

Tell us on Discord (https://discord.gg/XQtM5TmNs8) or open an issue on the project's page, with where in the
game it happened, a screenshot if you can, and the latest files in the logs folder (the game's and, if the
launcher misbehaved, the launcher's).


KNOWN ISSUES

- Guitar Hero: Aerosmith runs on Guitar Hero III's engine, with GHMetalRecomp's fixes. Songs have been played
  with a guitar and with a controller; songs with two players (co-op and battle) have not been tested yet.
- With some controllers the whammy bar reads partly pressed (about 40 %) until you first move it.
- In the opening video, about a second of colored lines at the top of the screen as the Neversoft logo
  comes in.
- Some USB audio interfaces add delay in Windows' shared audio mode (a Focusrite Scarlett about 20 ms more
  than a motherboard's audio): calibrate the lag in the game's options.
- Above 60 FPS is experimental: something may still run too fast. Tell us what and where.
- Playing with the keyboard is experimental.
- Your highway pictures and custom textures need NativeXRenderer (the default renderer).
- The first time an effect appears, it may show up a few frames late.
- Xbox One guitars (PDP Riffmaster, PDP Jaguar, Mad Catz Stratocaster) have not been played with yet: tell us
  how yours goes, with the game's latest log.


MORE INFORMATION

{MORE_INFO}

GHAeroRecomp's own code is under the MIT license (LICENSE). It is built on the ReXGlue SDK and other
components under their own licenses: see THIRD_PARTY_NOTICES.md and the "licenses" folder.
