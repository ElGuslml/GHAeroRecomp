# Building and developing GHAeroRecomp

GHAeroRecomp is a static recompilation of **Guitar Hero: Aerosmith** (Xbox 360, 2008) to native Windows, built on the
[ReXGlue SDK](https://github.com/rexglue/rexglue-sdk). The game's PowerPC code is translated to C++ and compiled. It is
drawn by [NativeXRenderer](native-x-renderer-ghaerosmith.md) (Vulkan), which draws the game's Direct3D calls on the PC's
GPU; ReXGlue's emulation of the Xbox 360 GPU, Xenos (derived from [Xenia](https://xenia.jp)), is the alternative. The
game runs on Guitar Hero III's engine (its executable is `gh3_xp1_Final.exe`), the one before Guitar Hero: Metallica's,
and [GHMetalRecomp](https://github.com/ElGuslml/GHMetalRecomp)'s fixes were ported to it, each site found again
([`fixes-from-gh-metallica.md`](fixes-from-gh-metallica.md)); the other notes in [`docs/`](README.md) describe each
problem found and what was done about it.

## Requirements

- Git, [CMake](https://cmake.org/) 3.25 or newer, [Ninja](https://ninja-build.org/),
  [LLVM/Clang](https://github.com/llvm/llvm-project/releases) (MSVC alone does not build this) and the Windows SDK.
- Python 3 (glslang needs it to configure the Vulkan backend).
- Your own copy of the game: extract the content of your disc or ISO with
  [extract-xiso](https://github.com/XboxDev/extract-xiso/releases) and copy all of it into `assets/`, so that
  `assets/default.xex` exists (title ID `41560819`, version 0.0.0.1, XDK 2.0.6683). The expected executable has
  CRC32 `529115C6`; `GHAerosmithRecomp.exe` warns at startup if yours is a different version. See
  [`assets/README.md`](../assets/README.md).

## Building

```powershell
git clone --recursive https://github.com/ElGuslml/GHAeroRecomp
cd GHAeroRecomp
# copy your game files into assets\ first: the build recompiles assets\default.xex
cmake --preset win-amd64-release
cmake --build --preset win-amd64-release
```

- If you cloned without `--recursive`: `git submodule update --init --recursive`.
- The first build translates `default.xex` into C++ (`generated/`) and builds the SDK: it takes a while. If it stops
  with "precompiled header ... has been modified", run the build command again.
- Presets: `win-amd64-release`, `win-amd64-relwithdebinfo`, `win-amd64-debug`.

## Running

```powershell
.\out\build\win-amd64-release\"GH Aerosmith Launcher.exe"   # choose the options, then Play
.\out\build\win-amd64-release\GHAerosmithRecomp.exe          # or start the game directly with the saved options
```

- Game files are found in `assets\` next to `GHAerosmithRecomp.exe`, otherwise in the repository's `assets\`.
  `--game_data_root=<folder>` points somewhere else. `"GH Aerosmith Launcher.exe" --launcher_first_run` installs them
  again from an ISO.
- The saves, the achievements and the playtime go to the current profile of the launchers' shared store
  (`%APPDATA%\ElGuslml Recomps`); the shader cache to `userdata\` next to `GHAerosmithRecomp.exe` (if that folder
  cannot be written, for example under Program Files, to `Documents\ghaerosmith`). Logs go to `logs\`.

## Configuration

| File (next to `GHAerosmithRecomp.exe`) | What it holds |
|---|---|
| `ghaerosmith.toml` | Base settings. Created with the defaults if it is missing; a default setting the file lacks is appended at startup. |
| `ghaerosmith_options.toml` | What GH Aerosmith Launcher saves. Loaded after `ghaerosmith.toml`. |

Command-line options (`--name=value`, for example Steam launch options) win over both files, for example
`--fullscreen=false --window_width=1920 --window_height=1080`, `--native_x_renderer=false` (the Xenos path),
`--ghaerosmith_frame_rate=120` (60, 72, 75, 90, 100, 120, 144, 160, 165 or 240, and 300, 360, 480 or 1000 with V-Sync
off; [frame rates above 60](fixes-from-gh-metallica.md#frame-rates-above-60)), `--ghaerosmith_ultrawide=true`
([ultrawide](fixes-from-gh-metallica.md#ultrawide)), `--anisotropic_override=0` (off; 5 = 16×), `--user_language=4`
(French; [language](fixes-from-gh-metallica.md#language)), `--quit_prompt=false` (B on the title screen and in the
main menu does not ask to quit) and `--gpu_adapter=10DE:2782` (the graphics card by its PCI vendor and device IDs in
hexadecimal, as the launcher's Settings → Graphics writes it; `""` = automatic). The mods' and the screen effects'
switches are in [`mods.md`](mods.md) and [`screen-effects.md`](screen-effects.md).

## The launcher

GH Aerosmith Launcher is [RecompLauncher](https://github.com/ElGuslml/RecompLauncher) (`recomp_launcher/`, a
submodule: the launcher of every one of these recompilations, SDL3 + Dear ImGui) with this game's profile in
`launcher/`: `launcher/game.h` (names, the disc, the look, the options), `launcher/game.cpp` (the game's own texts),
`launcher/palette.h` (its colors) and `launcher/art/`, and its resources in `res/launcher.rc`.
`installer/ghaerosmith.iss` is its Inno Setup installer. The icon is `res/icon.svg` and `res/icon_small.svg`;
`res/make_icon.mjs` builds `res/icon.ico` from them, and `installer/make_images.ps1` the installer's images.

## How the SDK is modified

`thirdparty/rexglue-sdk` is [ElGuslml/rexglue-sdk](https://github.com/ElGuslml/rexglue-sdk), branch `ghmetal` (the
`recomp` line and the changes made since for the Guitar Hero games): the official ReXGlue SDK's nightly of 2026-10-02
(v0.10.0 and 24 later commits) with the changes this project, the other Guitar Hero recompilations, KameoRecomp,
GRAWRecomp (Ghost Recon Advanced Warfighter), POTCAWERecomp (Pirates of the Caribbean: At World's End) and
[NativeXRenderer](https://github.com/ElGuslml/NativeXRenderer) need, one commit each. None of the general fixes is
specific to one game, and the options they add are off by default (this game sets the ones it needs in
`src/startup.cpp`); the ones for NativeXRenderer change nothing while it is off. The branch's `RECOMP.md` lists them,
and each commit message explains its change.

`native_x_renderer/` is NativeXRenderer, a submodule: a native renderer that hooks the game's Direct3D library. This
game's side of it (its hooks and the map of the game's Direct3D library) is in
`src/native_x_renderer_ghaerosmith.cpp` and [`native-x-renderer-ghaerosmith.md`](native-x-renderer-ghaerosmith.md).

## Contributing: branches and pull requests

| Branch | What it is |
|---|---|
| `main` | Releases only (tags `vX.Y.Z`). |
| `dev` | The integration branch: it moves only by merges (a finished group of features, a pull request, a release's version commit). **Base pull requests on `dev`.** |
| `feat/<topic>` | Work in progress. Merged into `dev` once tested. |
| `fix/<topic>` | A fix players need now, merged into `dev` at once. |

**The submodules.** NativeXRenderer, RecompLauncher and the SDK fork share the branch `ghmetal` with the other
recompilations. A group that changes a submodule uses the same `feat/<topic>` there; when the group is done, that branch
is merged into `ghmetal` first, the game's branch records the merge, and the game's branch is merged into `dev`. A pull
request to a submodule targets `ghmetal`, and the game's pull request records the submodule pull request's commit.

**Rules that keep conflicts small:**
- **Numbers given at merge, in the order merged:** NativeXRenderer's decisions (`docs/decisions.md`), the SDK's changes
  (`RECOMP.md`) and RecompLauncher's version (its `CHANGELOG.md`).
- **Hooks:** `[[midasm_hook]]`s, functions and switch tables go in `ghaerosmith_functions.toml`, each with a comment
  that says what it is. Only one hook can sit at an address: search the address first.
- **After every codegen** (a change to `ghaerosmith_functions.toml` or the code generator), run recomp-tools'
  `tools/codegen/check_switch.mjs` (every switch table decoded) and `tools/codegen/msr_sites.py --project <repo>`
  ([`interlocked-without-lock.md`](interlocked-without-lock.md)).
- **Settings:** a new option is off by default and leaves the game as it was; its default goes in `src/startup.cpp`.
- **In the description:** what was tested (hardware, settings, the game's log), and what was not.
