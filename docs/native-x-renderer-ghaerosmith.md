# NativeXRenderer in Guitar Hero: Aerosmith

The game's side of NativeXRenderer (`native_x_renderer/`, the library that hooks a game's Direct3D calls and draws
on the host). The hooks and the profile are in
[`src/native_x_renderer_ghaerosmith.cpp`](../src/native_x_renderer_ghaerosmith.cpp); the library's checklist is
`native_x_renderer/docs/integrating-a-game.md`.

**Status (2026-10-27):** mapped and checked with the observer; off by default until a full play test goes through
it. Unlike GH Van Halen, this is not GH Metallica's engine: Guitar Hero III's (the executable is `gh3_xp1_Final.exe`),
built with another XDK, so the D3D library was mapped again, function by function, from GH Metallica's map.

## The addresses

- The XEX imports XDK 2.0.6683; the D3D library itself passes 6534 to `VdInitializeEngines`, where the other games
  pass their XDK version. It is at about `0x824C0110`–`0x824F75B8`, the draws inside it (`0x824DFAB0`–`0x824E1814`),
  and the XGraphics-style helpers (the texture unlocks, the headers) inside it too (`0x824CB700`–`0x824CE058`).
- Each of GH Metallica's 36 hooked functions has its equivalent here, found by reading both (a function matcher gave
  the first ones; the rest by their callers and the device fields they touch). 35 are hooked: BeginIndexedVertices
  (`sub_824DFFA8`) is only reached from DrawIndexedVerticesUP, which the engine never calls. The four other texture
  types' unlocks (volume, cube, array, line: the engine's texture filter reaches them) are hooked as TextureUnlock.
- The device pointer is at `0x82BE7820` (the renderer's block `0x82BE6D30` + 2800); the device is 22400 bytes. Its
  layout is GH Metallica's up to the flag bytes (`+10943`), 4 bytes less from about `+10948` to the vertex
  declaration (`11820`), 8 less for the current objects and the tiling state (`+11832` to `+13188`: the render
  targets at `12440`, the textures at `12544`, the viewport at `12648`), and GH Metallica's again from `+13200` on.
  The resource objects are GH Metallica's.
- The engine never calls BeginVertices or EndVertices itself: only DrawVerticesUP commits BeginVertices' packets, as
  in POTCAWE, so the profile leaves `inline_draw_end` unset. It tiles only in the renderer's setup (once per front
  buffer), never in a frame.
- `scene_resolve_caller = 0x823BF160`: the return address of the resolve of the finished 1280x720 scene, once a frame,
  before the bloom's 320x180 passes (the trace of a song's frame: "from 0x823bf160 < 0x82380b40"); NativeXRenderer
  applies its antialiasing (SMAA) to the scene there.

## Checked

- 2026-10-27, the observer (`--native_x_renderer_verify=true`, F10): the main menu 221 of 221 draws and a Quickplay
  song 365 of 365 draws fully match the state the library rebuilt (100 %); no render state, constant, fetch or shader
  mismatches, no memexport, no tiling.
- `--native_x_renderer=true` at 1280x720: the menus and a song drawn right (Vulkan, 60 FPS, SMAA), 194-231 draw calls a
  frame, none failed, GPU 1.6 ms a frame. At 1720x720 (21:9) with the ultrawide hooks (`widens_cameras`): the stage over
  the whole width, the highway and the HUD at 16:9, the menu's bars.

## The stage dark for a frame in songs

**Symptom:** with NativeXRenderer, now and then in a song the stage went dark for one frame (the band and the lights
gone, the HUD intact): about one frame in 450, at 60 and 160 FPS. **Cause:** the engine kicks command buffer segments
outside the D3D calls NativeXRenderer hooks. A kick writes its marks and moves the write pointer on to the next 32
bytes; the dwords in between were never written and hold packets from the ring's last lap. NativeXRenderer read them as
packets, and now and then they were an old write of the window scissor (the bloom's 320x180 pass): the scene's draws were
clipped to it. **Fix:** NativeXRenderer's D102 (its `docs/decisions.md`): a segment change outside a hooked call is not
read. Checked: no dark frame in a whole song (25-43 before) and in 1,000 screenshots (3-7 before); the observer at
100 %. There is no switch for it; `--native_x_renderer=false` is the Xenos path, which never had it.

Left: the crowd's sheets, six 256x512 textures, are resolved into mip levels 1-3 each frame (18 resolves); the library
resolves level 0 only, as in GH Metallica, so far away they are sampled at level 0.

## The bloom's reduced copies at 1x (D127)

The post-processing blurs reduced copies of the 1280x720 scene, as GH Metallica's: 320x180 targets for the glow and
the blurs, whose tap offsets are constants for 320x180 texels. Above a render scale of 2 those offsets skip host
texels, which shows as a grid of dots in the glow (GH Metallica's lattice). The profile sets
`unscaled_screen_fractions` (2026-11-24): the 320x180 targets and the textures resolved through them stay at 1x, as
on the console; the log says "native: a reduced copy of the screen, D127". At 1440p (scale 2) it changes nothing seen.
**Off:** `native_x_renderer_unscaled_screen_fractions = false`.
