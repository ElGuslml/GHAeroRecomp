// GH Aerosmith: D3D's fence stamps written where the renderers' page watches do not see them (GH Warriors of Rock's and
// GH5's src/fence_stamps.cpp, GH Warriors of Rock's docs/fence-stamps.md; the same D3D code, other addresses).
//
// When SetStreamSource (sub_824D1070), SetIndices (sub_824D1218) or SetTexture (sub_824CE078) unbinds a vertex buffer,
// an index buffer or a texture, D3D writes the device's current fence (+10908 of the device, as in GH Warriors of Rock,
// GH Metallica and GH5) into the resource's header (resource + 8), to know later when the GPU is done with it. The
// game keeps those headers in physical memory, next to vertex data and inside textures' packed mip tails, and the
// renderers watch physical memory for the game's writes: each stamp on a watched page faults (on the game's render
// thread) and invalidates the page, which is then uploaded, protected or hashed again; a page stamped every frame is
// left unwatched (NativeXRenderer D97) and compared at every request. In GH Warriors of Rock 85 % of the writes that
// faulted in a song were the first two stores; at 240 FPS NativeXRenderer's copies into its mirror went from 644 to 231
// a frame and its renderer from 3.65 to 3.00 ms a frame.
//
// The hooks write the stamp through the host view of physical memory the watches never protect (the one the SDK's data
// providers write through) and skip the game's store: the same bytes in the same memory, read back by D3D's own checks
// through its view, without a fault or an invalidation. No GPU work reads a resource's header; a real write to the page
// (new vertex data) still faults and uploads it, the current stamp with it. GH5's other stamp of the same family (a
// render object's last drawn frame) is engine code: not looked for here.
// ghaerosmith_fence_stamps_unwatched = false stores as the game does.

#include <rex/cvar.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <bit>
#include <cstdint>

REXCVAR_DEFINE_BOOL(ghaerosmith_fence_stamps_unwatched, true, "GHAerosmith",
                    "The fence D3D writes into a vertex buffer's, index buffer's or texture's header when "
                    "SetStreamSource, SetIndices or SetTexture unbinds it is written where the renderers' page watches "
                    "do not see it: no fault, and the page is not uploaded or compared again for it. false = the game's "
                    "own store");

namespace {

// Writes `value` at `address` (guest virtual) through the physical view; false if it is not in physical memory (no
// watch there: the game's store does it).
bool StoreUnwatched(uint32_t address, uint32_t value) {
  if (!REXCVAR_GET(ghaerosmith_fence_stamps_unwatched)) {
    return false;
  }
  auto* kernel = rex::system::kernel_state();
  rex::memory::Memory* memory = kernel ? kernel->memory() : nullptr;
  if (!memory) {
    return false;
  }
  if ((address & 3) != 0) {
    return false;
  }
  const uint32_t physical = memory->GetPhysicalAddress(address);
  if (physical == UINT32_MAX) {
    return false;
  }
  *memory->TranslatePhysical<uint32_t*>(physical) = std::byteswap(value);  // guest memory is big-endian
  return true;
}

}  // namespace

// stw r11,8(r28) at 0x824D1100 (SetStreamSource: the stream's previous vertex buffer).
bool GHAerosmithStreamFenceHook(PPCRegister& r11, PPCRegister& r28) {
  return StoreUnwatched(r28.u32 + 8, r11.u32);
}

// stw r11,8(r30) at 0x824D1244 (SetIndices: the previous index buffer).
bool GHAerosmithIndicesFenceHook(PPCRegister& r11, PPCRegister& r30) {
  return StoreUnwatched(r30.u32 + 8, r11.u32);
}

// stw r11,8(r27) at 0x824CE194 (SetTexture: the sampler's previous texture).
bool GHAerosmithTextureFenceHook(PPCRegister& r11, PPCRegister& r27) {
  return StoreUnwatched(r27.u32 + 8, r11.u32);
}
