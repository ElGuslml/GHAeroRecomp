// ghaerosmith - NativeXRenderer for Guitar Hero: Aerosmith: the game's D3D entry points and its D3D device and
// object layouts.
//
// The XDK Direct3D library is linked into default.xex at about 0x824C0110-0x824F75B8, the draws included
// (0x824DFAB0-0x824E1814), with XGraphics-style helpers inside it (0x824CB700-0x824CE058: the texture unlocks, the
// texture and surface headers sub_824CD0C0 and sub_824CD3D0) and more after it (0x824F75B8 on). The XEX imports
// XDK 2.0.6683; the library itself passes 6534 to VdInitializeEngines (0x19860001, sub_824E7B98
// 0x824E7BB8-0x824E7BD0), where the other games pass their XDK version. The device pointer is [0x82BE7820] (the
// renderer block 0x82BE6D30 + 2800). The device layout is GH Metallica's (XDK 7978) up to the flag bytes (+10943);
// from about +10948 to the vertex declaration it is GH's - 4, the current objects and the tiling state (+11832 to
// +13188) GH's - 8 (POTCAWE's + 136), then GH's again from the tiling clear values (+13200) on; the device is 22400
// bytes (GH 24064). The resource objects are GH's and POTCAWE's. Every address and offset here, with its evidence,
// goes in docs/native-x-renderer-ghaerosmith.md (TODO: not written yet; the static analysis' map is its start). The
// hooks only run the original function unless a NativeXRenderer feature is on (--native_x_renderer_trace,
// --native_x_renderer_verify, --native_x_renderer).

#include "native_x_renderer_ghaerosmith.h"

#include <native_x_renderer/hooks.h>

// Draws. The engine calls DrawIndexedVertices (14 sites), DrawVertices (20) and DrawVerticesUP (25); BeginVertices
// is called only by DrawVerticesUP, which inlines the end ([dev+48] = [dev+13444]), as in POTCAWE. Linked but never
// called (no call, no address built in code, no pointer in the image but .pdata entries): EndVertices
// (sub_824DFF50, and its copy sub_824E04D8), DrawIndexedVerticesUP (sub_824E04E8) and so BeginIndexedVertices
// (sub_824DFFA8), and two tessellated draws (sub_824E0DC8, sub_824E1240: VGT_OUTPUT_PATH_CNTL and the VGT_GROUP_*
// registers). None of them is hooked, and the profile leaves inline_draw_end unset.
NATIVE_X_RENDERER_HOOK(824E0970, DrawIndexedVertices)
NATIVE_X_RENDERER_HOOK(824E0588, DrawVertices)
NATIVE_X_RENDERER_HOOK(824DFF60, DrawVerticesUP)  // calls BeginVertices, inlines the end
NATIVE_X_RENDERER_HOOK(824DFAB0, BeginVertices)

// Render targets, clears, resolves, tiling. The engine sets targets through sub_824D2980, an alias that branches to
// SetRenderTarget (32 sites). The engine tiles only in the renderer's setup (sub_822C56D0, once per front buffer,
// when [0x82BE6D30+11621] is set): BeginTiling, Clear of depth and stencil, then EndTiling with flags 0, the tile
// rects at 0x82BE6D30+3160 and the front buffer as the destination (EndTiling resolves the tiles itself),
// Synchronize, Present. Its frames do not tile.
// BeginZPass (sub_824D4DD8) and EndZPass (sub_824D5120) are linked, unlike GH's, but never called.
NATIVE_X_RENDERER_HOOK(824D1CE0, SetRenderTarget)
NATIVE_X_RENDERER_HOOK(824D2048, SetDepthStencilSurface)
NATIVE_X_RENDERER_HOOK(824D3FF8, Clear)  // calls ClearF
NATIVE_X_RENDERER_HOOK(824D3ED0, ClearF)
NATIVE_X_RENDERER_HOOK(824DA430, Resolve)
NATIVE_X_RENDERER_HOOK(824D4480, BeginTiling)
NATIVE_X_RENDERER_HOOK(824D4918, EndTiling)
NATIVE_X_RENDERER_HOOK(824D4308, SetPredication)
NATIVE_X_RENDERER_HOOK(824D1BA8, SetViewport)  // D3DVIEWPORT9 (integers), stored as floats at +12648
NATIVE_X_RENDERER_HOOK(824D0EE0, SetScissorRect)

// Shaders, vertex input, textures, constants. The engine writes constants straight into the device too (VS in 25
// functions, PS in 28) and patches sampler states in the fetch constants (22 functions), without a call.
// SetTexture, SetStreamSource and the constant setters take the dirty bit or mask from their caller.
NATIVE_X_RENDERER_HOOK(824C4B20, SetVertexShader)
NATIVE_X_RENDERER_HOOK(824C4820, SetPixelShader)
NATIVE_X_RENDERER_HOOK(824C4D40, SetVertexDeclaration)
NATIVE_X_RENDERER_HOOK(824D1070, SetStreamSource)
NATIVE_X_RENDERER_HOOK(824D1218, SetIndices)
NATIVE_X_RENDERER_HOOK(824CE078, SetTexture)
NATIVE_X_RENDERER_HOOK(824C42B8, SetVertexShaderConstantF)
NATIVE_X_RENDERER_HOOK(824C4398, SetPixelShaderConstantF)

// Resources: typed thunks over the common Lock (sub_824DB770) and Unlock (sub_824DB9A8), and the cache flush over a
// range. The locks return the pointer (no data argument: the engine's wrappers store it) and ignore the offset and
// size. The library has one unlock thunk per texture type, all with the same body: Texture (sub_824CB850, the one
// the engine calls, 27 sites), and VolumeTexture, CubeTexture, ArrayTexture and LineTexture, which only the
// engine's texture filter calls (sub_822CF740 -> sub_8250ED90 -> sub_8250BEA0, by D3DRESOURCETYPE 17-20). They
// are all reported as TextureUnlock (the trace reads the texture in r3). IndexBufferUnlock has an identical copy
// with no callers (sub_824DBF38).
NATIVE_X_RENDERER_HOOK(824DBDE0, VertexBufferLock)
NATIVE_X_RENDERER_HOOK(824DBEE0, VertexBufferUnlock)
NATIVE_X_RENDERER_HOOK(824DBFF8, IndexBufferLock)
NATIVE_X_RENDERER_HOOK(824DC040, IndexBufferUnlock)
NATIVE_X_RENDERER_HOOK(824CB850, TextureUnlock)
NATIVE_X_RENDERER_HOOK(824CB8C0, TextureUnlock)  // VolumeTexture (type 17)
NATIVE_X_RENDERER_HOOK(824CB880, TextureUnlock)  // CubeTexture (type 18)
NATIVE_X_RENDERER_HOOK(824CB8A8, TextureUnlock)  // ArrayTexture (type 19)
NATIVE_X_RENDERER_HOOK(824CB8D8, TextureUnlock)  // LineTexture (type 20)
NATIVE_X_RENDERER_HOOK(824F55E0, FlushCpuRange)

// Command buffer, queries, frame. The kick submits the segment and continues in the same memory; NewSegment moves the
// write pointer to a newly allocated segment (from the kick, the reservation sub_824DEE38 and Present). The engine's
// render thread (sub_822CE808) calls Synchronize (sub_824C0B38, Swap with no front buffer), then Present, which calls
// VdSwap with the front buffer in r4; the library's worker thread sub_824F33F8 can present again.
NATIVE_X_RENDERER_HOOK(824DEBB0, KickSegment)
NATIVE_X_RENDERER_HOOK(824DE7A8, NewSegment)
NATIVE_X_RENDERER_HOOK(824C5288, SetShaderGPRAllocation)
NATIVE_X_RENDERER_HOOK(824D72F0, QueryIssue)
NATIVE_X_RENDERER_HOOK(824D7578, QueryGetData)
NATIVE_X_RENDERER_HOOK(824C0698, Swap)
NATIVE_X_RENDERER_HOOK(824C0B40, Present)

namespace ghaerosmith {

const native_x_renderer::Profile& NativeXRendererProfile() {
  static const native_x_renderer::Profile profile = [] {
    native_x_renderer::Profile p;
    p.game = "Guitar Hero: Aerosmith (41560819)";
    p.xdk_version = "2.0.6683";  // the XEX's imports; the D3D library reports 6534 (see the top of this file)
    // The back buffer comes from a table at 0x8200ED70 (entries of 740 bytes, width at +4, height at +8): sub_822C4D28
    // calls XGetVideoMode and picks entry 11 (1280x720) when a configured width is 1280 or more, else 10 (1200x688),
    // 9 (1120x640), 8 (1040x584), 6 (960x544) or 4 (720x416).
    // TODO: confirm with the trace's Present (the front buffer's fetch constant) at the SDK's video mode.
    p.output_width = 1280;
    p.output_height = 720;
    // The Resolve that copies the finished 1280x720 scene (flags 0, into the texture 0x40066010) before the bloom: the
    // trace of a song's frame (ghaerosmith_020.log, F9) has it once, "from 0x823bf160 < 0x82380b40", after the 3D and
    // the highway's sprites in the 1280x720 target and before the 320x180 blur passes (from 0x823cce48) and the
    // composite. The other 1280x720 resolve of the frame (flags 0x14, from 0x823bf1cc < 0x822ed354) clears after it.
    p.scene_resolve_caller = 0x823BF160;
    p.scene_resolve_once = true;
    // The ultrawide hooks (src/widescreen.cpp) widen the 16:9 cameras by AspectFactor() and keep the 2D layer at 16:9.
    p.widens_cameras = true;
    // The pipeline list (D129): every shader the game has is in DATA/FXFILES/MaterialLibrary.bin.xen, as in GH Metallica (NativeXRenderer's D129, D133).
    static const char* const kShaderFiles[] = {"DATA/FXFILES/MaterialLibrary.bin.xen"};
    p.title_id = 0x41560819;
    p.shader_files = kShaderFiles;
    p.shader_file_count = 1;
    native_x_renderer::DeviceLayout& d = p.device;
    // Six dirty masks from +0, as in GH Metallica: VS constants, PS constants, register groups and the program, fetch
    // slots (low half, slot 0 in bit 31) and the 0x2280 group, the 0x2300/0x2380 groups with the clip planes and
    // bool/loop constants, the tile-dependent state. The draws' flush is GH's instruction for instruction
    // (DrawIndexedVertices 0x824E0990-0x824E0BA8).
    d.vs_constants_dirty = 0;
    d.ps_constants_dirty = 8;
    d.fetch_constants_dirty = 24;
    d.fetch_dirty_slot0_bit = 31;
    d.command_buffer_write = 48;
    d.fetch_constants = 1152;
    d.vs_float_constants = 1920;
    d.ps_float_constants = 6016;
    d.bool_loop_constants = 10112;
    d.register_groups = {{
        {10368, 0x2000, 16},
        {10444, 0x2100, 21},
        {10528, 0x2180, 5},
        {10548, 0x2200, 12},
        {10596, 0x2280, 21},
        {10680, 0x2300, 38},
        {10832, 0x2380, 8},
    }};
    d.fence_counter = 10908;
    d.write_back_block = 10896;
    d.flags = 10940;
    d.vertex_declaration = 11820;  // GH - 4
    // The current objects: GH's - 8.
    d.index_buffer = 12436;
    d.render_targets = 12440;
    d.depth_stencil = 12456;
    d.vertex_buffers = 12460;  // one pointer per stream
    d.textures = 12544;
    d.viewport = 12648;  // x, y, width, height, min z, max z (floats)
    d.pixel_shader = 12692;
    d.vertex_shader = 12696;
    d.predication_mask = 12708;
    d.tile_count = 12748;
    d.tile_rects = 12752;
    // inline_draw_end is left unset: only DrawVerticesUP commits BeginVertices' packets ([dev+48] = [dev+13444], the
    // same field as GH's), and it is hooked. Set it to 13444 only if the trace finds an inline draw the engine commits.
    native_x_renderer::ObjectLayout& o = p.objects;
    o.texture_fetch = 28;
    o.surface_info = 24;
    o.surface_color_depth_info = 28;
    o.surface_size = 36;
    o.surface_width_minus_one = {18, 14};
    o.surface_height_minus_one = {3, 15};
    o.buffer_address = 24;
    o.buffer_size = 28;
    return p;
  }();
  return profile;
}

}  // namespace ghaerosmith
