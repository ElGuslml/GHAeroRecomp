# Documentation

Short notes on each problem found while making the game run, and what was done about it. Each one gives
the symptom, the cause, the fix and how to turn the fix off, so that you can check it on your own machine.

Guitar Hero: Aerosmith runs on Guitar Hero III's engine (its executable is `gh3_xp1_Final.exe`), the one before Guitar
Hero: Metallica's. Many of GH Metallica's problems are here too: GHMetalRecomp's fixes were ported, each site found
again in this engine; the investigations are GHMetalRecomp's.

| Note | Status |
|---|---|
| [The fixes ported from GH Metallica](fixes-from-gh-metallica.md) | Ported: the guitar, the threads' waits, the song clock, the audio, the crowd's claps, quitting, the second controller, the keyboard, frame rates above 60, ultrawide, language; 2026-11-13: the intro skip, the red border, V-Sync and the SDK's settings, the render thread woken by the counters (G-Sync), a controller on the game's own layout, the launcher 3.21 |
| [NativeXRenderer in Guitar Hero: Aerosmith](native-x-renderer-ghaerosmith.md) | Working: the D3D library mapped again (XDK 2.0.6683), the observer at 100 % (off by default) |
| [Stutters the first time something is shown](compiling-shaders.md) | Fixed: a pipeline list ships with the game; the launcher compiles it for your graphics card at the first Play ("Compiling shaders"; NativeXRenderer) |
| [D3D's fence stamps](fence-stamps.md) | Fixed (2026-11-24): the fences Direct3D writes into unbound buffers and textures go where the page watches do not see them; a third fewer buffer copies a frame in a song (NativeXRenderer) |
| [The XDK's atomic operations without a process-wide lock](interlocked-without-lock.md) | Fixed (2026-11-24): `skip_msr`, every site checked; about 16 % less CPU in a song at 160 FPS |
| [The keyboard](keyboard.md) | Fixed (2026-11-24): a guitar's keys in the menus (D selects), the game's controller layout in a song |
| [Pending](pending.md) | What GH Metallica has that this game does not yet, and why: the audio's pacing, the debug menu, the screen effects, a look at G-Sync each controller's kind, NativeXRenderer checks, online, the release |
