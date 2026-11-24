# D3D's fence stamps written where the page watches do not see them

**Symptom.** None on screen: work. Each frame NativeXRenderer copied guest memory again that had not really changed
(its log's line "per frame, N buffer copies average"), and the game's render thread took a page fault for each
stamp.

**Cause.** When SetStreamSource (`sub_824D1070`), SetIndices (`sub_824D1218`) or SetTexture (`sub_824CE078`) unbinds
a vertex buffer, an index buffer or a texture, Direct3D writes the device's current fence (+10908 of the device) into
the resource's header (+8), to know later when the GPU is done with it. Those headers sit in physical memory beside
vertex data and inside textures' packed mip tails, on pages both renderers watch for the game's writes: each stamp
faults and marks the page changed, which is then uploaded, hashed or compared again. The same code as in GH Warriors
of Rock, GH Metallica and GH5 (GHWoRRecomp's `docs/fence-stamps.md`), at other addresses.

**Fix** (`src/fence_stamps.cpp`, three `[[midasm_hook]]`s in `ghaerosmith_functions.toml` at the stores
`0x824D1100`, `0x824D1244`, `0x824CE194`). The hook writes the same value through the host's view of physical memory,
which the watches never protect, and skips the game's store: the same bytes, read back by D3D's own checks, without
a fault. No GPU work reads a resource's header; a real write to the page still faults and uploads it.

Measured on 2026-11-24 (NativeXRenderer, 160 FPS, 1280x720, Dream Police on Easy with nobody playing; the switch on
against off, two runs of the same build):

| | On | Off |
|---|---|---|
| Buffer copies a frame, menus | ~115 | ~228 |
| Buffer copies a frame, the song (average) | 243 | 388 |
| Buffer copies a frame, the song's results screen | 321 | 504 |
| Guest memory compared a frame, results screen | 4.3 MB | 6.7 MB |
| Pages left unwatched (rewritten every frame) | ~3,720 | ~4,460 |

**Turn it off:** `ghaerosmith_fence_stamps_unwatched = false` in `ghaerosmith.toml` (the game's own stores).
