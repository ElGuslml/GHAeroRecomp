// ghaerosmith - ultrawide displays (see widescreen.h).
//
// Ported from GH Metallica's src/widescreen.cpp (2026-10-27). The engine is older (Guitar Hero III's) and draws its 2D
// layer differently; the hooks below were found by their role, not by matching functions. Checked at 1720x720 (21:9)
// on both renderers: the main menu at 16:9 with the bars, a Quickplay song with the stage widened and the highway, its
// gems and the HUD at 16:9. The TODOs are what that run did not show.

#include "widescreen.h"

#include <imgui.h>
#include <native_x_renderer/native_x_renderer.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>
#include <rex/ui/imgui_dialog.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>

REXCVAR_DEFINE_BOOL(ghaerosmith_menu_bars, true, "GHAerosmith",
                    "With ultrawide: black bars at the sides outside a song (the menus, the loading screens and the "
                    "videos were made for 16:9); in a song the stage fills the screen");
REXCVAR_DEFINE_BOOL(ghaerosmith_ultrawide, false, "GHAerosmith",
                    "Ultrawide on the Xenos path: the game's 3D views see as much more at the sides as the window is "
                    "wider than 16:9 (anamorphic, the presenter stretches the 1280x720 image; needs "
                    "present_letterbox = false). With NativeXRenderer, native_x_renderer_fill_screen does this instead");
REXCVAR_DEFINE_BOOL(ghaerosmith_ultrawide_overlays, true, "GHAerosmith",
                    "With ultrawide, in a song: a 2D overlay over the whole screen (a red border while the band is "
                    "failing) covers the whole width; false = kept at 16:9 in the center with the HUD");
REXCVAR_DEFINE_BOOL(ghaerosmith_ultrawide_hud, true, "GHAerosmith",
                    "With ghaerosmith_ultrawide: keep the HUD, the menus and the videos at 16:9 in the center "
                    "(false = stretched)");

namespace ghaerosmith {
namespace {

constexpr double kGuestAspect = 16.0 / 9.0;

// Factor the 16:9 views are widened by (1 = unchanged).
std::atomic<double> g_aspect_factor{1.0};

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

// When the note highway's board and a picture (a movie, GHAerosmithElementQuadHook, or the loading screen,
// GHAerosmithLoadingScreenHook) were last drawn (steady clock ticks, 0 if never). The board is the sprite drawn as a
// grid (mesh type 1 of the sprite draw, sub_82390628): in a song's frame it is drawn once (1632 vertices, the trace of
// frame 4508). TODO: as in GH Metallica (runs 264, 266, 269), check that it is drawn in every frame of a song from its
// first frame and while it is paused, never in the menus, and whether a song's first frames run under its loading
// screen.
std::atomic<int64_t> g_highway_drawn{0};
std::atomic<int64_t> g_picture_drawn{0};

bool DrawnRecently(const std::atomic<int64_t>& mark) {
  int64_t last = mark.load(std::memory_order_relaxed);
  auto now = std::chrono::steady_clock::now().time_since_epoch();
  return last != 0 && now.count() - last <
                          std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(250)).count();
}

bool InSong() { return DrawnRecently(g_highway_drawn) && !DrawnRecently(g_picture_drawn); }

void MarkDrawn(std::atomic<int64_t>& mark) {
  mark.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_relaxed);
}

// Black over the sides of the image, where the 2D layer is not (the 16:9 area in the middle, as wide as the image over
// the factor), while no song is drawn. The background draw list: under any dialog (the achievements' toasts).
class MenuBarsDialog : public rex::ui::ImGuiDialog {
 public:
  explicit MenuBarsDialog(rex::ui::ImGuiDrawer* drawer) : ImGuiDialog(drawer) {}
  bool WantsContinuousPaint() override { return false; }

 protected:
  void OnDraw(ImGuiIO& io) override {
    double factor = g_aspect_factor.load(std::memory_order_relaxed);
    if (factor <= 1.0 || !REXCVAR_GET(ghaerosmith_menu_bars) || InSong()) {
      return;
    }
    float width = io.DisplaySize.x, height = io.DisplaySize.y;
    float bar = std::floor(float(width * (1.0 - 1.0 / factor) * 0.5));
    if (bar <= 0.0f) {
      return;
    }
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(bar, height), IM_COL32(0, 0, 0, 255));
    draw->AddRectFilled(ImVec2(width - bar, 0.0f), ImVec2(width, height), IM_COL32(0, 0, 0, 255));
  }
};

}  // namespace

void SetOutputSize(uint32_t width, uint32_t height) {
  double factor = 1.0;
  if (native_x_renderer::Renders()) {
    // NativeXRenderer renders at the output's aspect ratio when it fills it (the profile says these hooks widen the
    // cameras): the hooks follow its factor, whatever ghaerosmith_ultrawide says. They handle wider outputs only.
    // TODO: the profile's widens_cameras (src/native_x_renderer_ghaerosmith.cpp) becomes true with these hooks.
    double native = native_x_renderer::AspectFactor();
    if (native > 1.0) {
      factor = native;
    } else if (native < 1.0) {
      static std::atomic<bool> warned{false};
      if (!warned.exchange(true)) {
        REXLOG_WARN("[ultrawide] NativeXRenderer fills an output taller than 16:9 ({:.4f}); the game's hooks only "
                    "widen views, so the image is stretched vertically (native_x_renderer_fill_screen = false "
                    "keeps 16:9 with bars)",
                    native);
      }
    }
  } else if (REXCVAR_GET(ghaerosmith_ultrawide) && width > 0 && height > 0) {
    double aspect = double(width) / double(height);
    if (aspect > kGuestAspect) {
      factor = aspect / kGuestAspect;
    }
  }
  if (g_aspect_factor.exchange(factor) != factor) {
    REXLOG_INFO("[ultrawide] output {}x{}: aspect factor {:.4f}", width, height, factor);
  }
}

std::unique_ptr<rex::ui::ImGuiDialog> CreateMenuBars(rex::ui::ImGuiDrawer* drawer) {
  return drawer ? std::make_unique<MenuBarsDialog>(drawer) : nullptr;
}

}  // namespace ghaerosmith

namespace {

float LoadFloat(const uint8_t* base, uint32_t address) {
  uint32_t bits;
  std::memcpy(&bits, base + address, 4);
  bits = __builtin_bswap32(bits);
  float value;
  std::memcpy(&value, &bits, 4);
  return value;
}

void StoreFloat(uint8_t* base, uint32_t address, float value) {
  uint32_t bits;
  std::memcpy(&bits, &value, 4);
  bits = __builtin_bswap32(bits);
  std::memcpy(base + address, &bits, 4);
}

uint32_t LoadWord(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

void StoreWord(uint8_t* base, uint32_t address, uint32_t value) {
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

}  // namespace

namespace {

// The D3D device ([0x82BE7820], the renderer block 0x82BE6D30 + 2800) and the fields the hooks read: render target 0
// (+12440, GH Metallica's +12448: SetRenderTarget sub_824D1CE0 stores slot +12440 + 4 * index) and a surface's size
// (+36: width - 1 in bits 18-31, height - 1 in bits 3-17), and the vertex shader's float constants (+1920, c0 first).
// The engine never calls BeginVertices / EndVertices itself, so there is no inline draw to read (GH Metallica's +13448
// and +13456).
constexpr uint32_t kDevicePointer = 0x82BE7820;
constexpr uint32_t kRenderTarget0 = 12440;
constexpr uint32_t kVsConstants = 1920;  // VS float constants, c0 first
constexpr uint32_t kSurfaceSize = 36;

// Whether the draw goes to the screen: render target 0 of 1280x720, not a texture. The 2D layer is drawn into the
// 1280x720 surface 0x40007820 after the post-processing, the 3D scene and the movies into another 1280x720 surface
// (0x400660b0); the post-processing's small targets (320x180) and the textures are not the screen.
bool DrawsToScreen(const uint8_t* base) {
  if (!base) {
    return false;
  }
  uint32_t device = LoadWord(base, kDevicePointer);
  uint32_t surface = device ? LoadWord(base, device + kRenderTarget0) : 0;
  if (!surface) {
    return false;
  }
  uint32_t size = LoadWord(base, surface + kSurfaceSize);
  uint32_t width = ((size >> 18) & 0x3FFF) + 1, height = ((size >> 3) & 0x7FFF) + 1;
  return width == 1280 && height == 720;
}

// The factor the 2D layer is narrowed by, or 1 when it is not: ultrawide on, its HUD option on, and the draw going to
// the screen.
double NarrowFactor(const uint8_t* base) {
  double factor = ghaerosmith::g_aspect_factor.load(std::memory_order_relaxed);
  if (factor == 1.0 || !REXCVAR_GET(ghaerosmith_ultrawide_hud) || !DrawsToScreen(base)) {
    return 1.0;
  }
  return factor;
}

// Whether the vertices (x and y the first two floats of each, pixels of the 1280x720 back buffer) are a piece of an
// overlay of the whole screen: at least a quarter of it (half its width and half its height) reaching one of its sides
// and its top or bottom (GH5's rule: its red border is four such quarters, or one quad over the canvas).
bool ScreenOverlayPiece(const uint8_t* base, uint32_t data, uint32_t count, uint32_t stride) {
  float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
  for (uint32_t i = 0; i < count; ++i) {
    const float x = LoadFloat(base, data + i * stride), y = LoadFloat(base, data + i * stride + 4);
    x0 = std::min(x0, x);
    x1 = std::max(x1, x);
    y0 = std::min(y0, y);
    y1 = std::max(y1, y);
  }
  return (x0 <= 0.5f || x1 >= 1279.5f) && (y0 <= 0.5f || y1 >= 719.5f) && x1 - x0 >= 639.5f && y1 - y0 >= 359.5f;
}

// The vertices' x (the first float of each, in pixels of the 1280x720 back buffer) narrowed around the center.
void NarrowVertices(uint8_t* base, double factor, uint32_t data, uint32_t count, uint32_t stride) {
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[ultrawide] 2D layer kept at 16:9 in the center ({:.4f})", factor);
  }
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t x = data + i * stride;
    StoreFloat(base, x, float(640.0 + (LoadFloat(base, x) - 640.0) / factor));
  }
}

}  // namespace

// The screen elements (the menus, the HUD, the note highway, its gems and their tails, the text) are drawn in pixels of
// the 1280x720 back buffer: sprites, text and clip windows go from the 1280x720 canvas to the back buffer through
// sub_8218FA20 / sub_8218FA50 (x = (x - canvas width / 2) * [0x82BE6D30 + 11300] + back buffer width / 2; the scale is
// back buffer width / canvas width = 1, set by sub_822C4E58). Their vertices are narrowed around the center just before
// they are drawn, so the whole 2D layer stays 16:9 in the middle of the stretched image, whatever its positions came
// from.
//
// [[midasm_hook]] at 0x823927D8 in sub_82391800 (the sprite's draw, its class's virtual draw at 0x8201CBE0, called by
// sub_823913D0 at 0x8239171C), a sprite's quad (mesh type 0, [sprite+88]): r6 = its 4 vertices (0x82CBBDA0), r7 =
// their stride (28, or 36 when [sprite+87] adds a mask's coordinates), before DrawVerticesUP (quad list). x is the
// first float. Seen: 78 a frame in a song (the trace of frame 4508, "from 0x823927dc < 0x82391720").
//
// In a song a sprite that is a piece of an overlay of the whole screen (a red border while the band is failing) is left
// as it is, so the overlay is stretched over the whole width instead of a frame around the middle 16:9
// (ghaerosmith_ultrawide_overlays; GH Metallica's, GH Van Halen's, GH5's and GH Warriors of Rock's rule).
void GHAerosmithSpriteHook(PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghaerosmith::GuestBase();
  double factor = NarrowFactor(base);
  if (factor == 1.0) {
    return;
  }
  if (REXCVAR_GET(ghaerosmith_ultrawide_overlays) && ghaerosmith::InSong() &&
      ScreenOverlayPiece(base, r6.u32, 4, r7.u32)) {
    static std::atomic<bool> logged{false};
    if (!logged.exchange(true)) {
      REXLOG_INFO("[ultrawide] a song's overlays over the whole screen cover the whole width");
    }
    return;
  }
  NarrowVertices(base, factor, r6.u32, 4, r7.u32);
}

// [[midasm_hook]] at 0x82369DBC in sub_82369928, a movie's frame (Bink: the Y, U, V and alpha planes as textures 0-3,
// shader 53, the same kind of quad as GH Metallica's sub_82465228): r6 = its 4 vertices on the stack (20 bytes each:
// x, y, z, u, v; a triangle strip), r7 = their stride, before DrawVerticesUP. Drawn by sub_8236A988 for a viewport's
// movie (the frame driver sub_822ED038 at 0x822ED4A8, into the viewport's 1280x720 target; TODO: which videos, the
// boot videos expected) and for the loading screen's movie mode (sub_82394BD0 at 0x82394E8C). Its position and size
// come from a sprite-like object (x [+40], y [+44], width and height [+48] / [+50], scales [+52] / [+56]), in pixels.
// Without it a video would be stretched over the whole screen.
// While a movie is drawn to the screen a song does not count as one for the bars (InSong). Unlike GH Metallica the
// mark needs the screen as the target: a movie drawn into a texture (TODO: whether a venue shows movies on its screens
// through a viewport texture override) must not put bars on a song.
void GHAerosmithElementQuadHook(PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghaerosmith::GuestBase();
  if (DrawsToScreen(base)) {
    ghaerosmith::MarkDrawn(ghaerosmith::g_picture_drawn);
  }
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    NarrowVertices(base, factor, r6.u32, 4, r7.u32);
  }
}

// [[midasm_hook]] at 0x82394E70 in sub_82394BD0, the loading screen (drawn on the render thread after the frame,
// sub_82386638 -> sub_82289450 while the byte 0x82BD9294 is set): reached in every frame it is drawn (its mode word
// 0x82CBC2DC is not 0), before sub_82394968 (which places four of its pieces). Its pictures and text are drawn through
// the screen elements' lists (sub_823D4AB0, three of them), or a movie (mode 4). GH Metallica's loading screen was a
// movie, marked by its element quad hook; here it is marked by itself. TODO: whether a song's first frames run under it
// (GH Metallica: run 266).
void GHAerosmithLoadingScreenHook() { ghaerosmith::MarkDrawn(ghaerosmith::g_picture_drawn); }

// GH Metallica's GHMetallicaParticleHook (sub_821B79F8, the 2D particles' own quads) has no counterpart: here the 2D
// particle systems (Create2DParticleSystem sub_82115C08, Update2DParticleSystems sub_8211E2F0) move screen elements
// (sub_82171760 looks them up in the element table [0x82A48430]), which are drawn as sprites (GHAerosmithSpriteHook).
// TODO: check the hit sparks at 21:9 in a song played with hits (none in the traced frame).

// [[midasm_hook]] at 0x8237C1A8 in sub_8237C138, a batch of quads (the text: sub_8237C560, the text's virtual draw,
// writes 4 vertices a glyph, 28 bytes each: x, y, z, w, color, u, v; flushed at 1024 vertices, at a texture change and
// at the element's end, sub_8237C1C0): r5 = the vertex count, r6 = the vertices (0x82CB30D8), r7 = their stride, before
// DrawVerticesUP (quad list).
void GHAerosmithSpriteBatchHook(PPCRegister& r5, PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghaerosmith::GuestBase();
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    NarrowVertices(base, factor, r6.u32, r5.u32, r7.u32);
  }
}

// [[midasm_hook]] at 0x8238F650 in sub_8238F4F8, a screen element's clip window (the sprites', sub_8238F928, and the
// text's, sub_8237C520, set before their draw; seen in a song for two HUD sprites): r4 = its rectangle {x1, y1, x2, y2}
// (ints, pixels of the back buffer: the window's x, y, x + w, y + h through sub_8218FA20 / sub_8218FA50), before
// SetScissorRect. The vertices it clips are narrowed around the center, so the rectangle is narrowed as well; left as
// it was, it would cut away what moved out of it. The engine does not clamp it (a window as large as the canvas is
// skipped instead); SetScissorRect intersects it with the viewport, as the scissor test is on (sub_824D1B98 just
// before). An empty window (x2 <= x1) stays empty.
void GHAerosmithClipWindowHook(PPCRegister& r4) {
  uint8_t* base = ghaerosmith::GuestBase();
  double factor = NarrowFactor(base);
  if (factor == 1.0) {
    return;
  }
  int32_t x1 = int32_t(LoadWord(base, r4.u32)), x2 = int32_t(LoadWord(base, r4.u32 + 8));
  int32_t n1 = int32_t(std::floor(640.0 + (x1 - 640.0) / factor));
  int32_t n2 = x2 > x1 ? int32_t(std::ceil(640.0 + (x2 - 640.0) / factor)) : n1;
  StoreWord(base, r4.u32, uint32_t(n1));
  StoreWord(base, r4.u32 + 8, uint32_t(n2));
}

// [[midasm_hook]] at 0x8239102C in sub_82390968 (mesh type 2 of the sprite draw: a sustained note's tail), a triangle
// strip, before DrawVerticesUP: r5 = the vertex count ((segments + 1) * 2, 60 in the traced frame), r6 = the vertices
// ([0x82CBBD90], 24 bytes each: x, y, color, ..., built by sub_8238FF70 every call), r7 = 24. Its vertices are in the
// lane's own space (x: plus or minus half its width, from the script values the tails read; y: the highway's rows),
// and its vertex shader places them on the canvas with VS constant c0, which sub_82390968 writes just before
// (0x82390D38-0x82390D50, from the sprite's quad and two script values): c0 = {X(far), far, 1 / (near - far),
// X(near) - X(far)}, X(y) the x of the quad's center line at y, the same layout as GH Metallica's, whose shader
// computes x = c0.x + x + c0.w * c0.z * (y - c0.y). TODO: the shader itself was not read; the layout is the evidence.
// Narrowing that x around the center: the vertices' x divided by the factor, c0.x moved toward 640, c0.w divided.
// The second draw of the same tail (0x8239120C, shader 76, when the element's value 0xAD551363 is not 0) reuses these
// vertices (only their y squeezed, 0x823910B8-0x823910F4) and this c0 (only c1 and a pixel shader constant rewritten):
// it is not hooked, or it would be narrowed twice.
void GHAerosmithTailHook(PPCRegister& r5, PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghaerosmith::GuestBase();
  double factor = NarrowFactor(base);
  if (factor == 1.0) {
    return;
  }
  for (uint32_t i = 0; i < r5.u32; ++i) {
    uint32_t x = r6.u32 + i * r7.u32;
    StoreFloat(base, x, float(LoadFloat(base, x) / factor));
  }
  uint32_t c0 = LoadWord(base, kDevicePointer) + kVsConstants;
  StoreFloat(base, c0, float(640.0 + (LoadFloat(base, c0) - 640.0) / factor));
  StoreFloat(base, c0 + 12, float(LoadFloat(base, c0 + 12) / factor));
}

// [[midasm_hook]] at 0x82390938 in sub_82390628 (mesh type 1 of the sprite draw: a sprite drawn as a grid of quads, the
// highway's board): r5 = the vertex count (sub_8238FAA8's result, 1632 in the traced frame), r6 = the vertices
// ([0x82CBBD8C], 20 bytes each: x, y, color, u, v, rebuilt every call from the sprite's quad and the highway's row
// tables), r7 = 20, before DrawVerticesUP (quad list). GH Metallica drew its board between BeginVertices and
// EndVertices; this engine draws it with DrawVerticesUP.
void GHAerosmithMeshSpriteHook(PPCRegister& r5, PPCRegister& r6, PPCRegister& r7) {
  ghaerosmith::MarkDrawn(ghaerosmith::g_highway_drawn);
  uint8_t* base = ghaerosmith::GuestBase();
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    NarrowVertices(base, factor, r6.u32, r5.u32, r7.u32);
  }
}

// [[midasm_hook]] at the entry of sub_8222F590, which builds a perspective projection from its frustum at the near
// plane (f1, f2 = left, right; f3, f4 = bottom, top; f5, f6 = near, far; m00 = 2n / (r - l), m20 = (l + r) / (r - l),
// m23 = -1: GH Metallica's sub_82356538, the right-handed form). Its callers take the frustum's width from the
// camera's horizontal field of view and its height from an aspect ratio: the main view's (sub_823ACEF0, from
// sub_823DF670) the SetScreen aspect at 0x82BBA148 (16:9 from 0x82009614 at the boot) unless it is 1, else the
// viewport's size [vp+56] / [vp+60]; the visibility setup's (sub_822D1D00 -> sub_823A8758, sub_823AADF0) the viewport's
// size; a wider screen would just stretch them. A 16:9 frustum is widened by the factor (Hor+: the vertical field of
// view kept, more seen at the sides), the same narrowing the 2D layer gets in the stretched image. Other shapes are
// left alone. TODO: log the aspect ratios seen here in a song and in the menus (sub_822AE2C8 and sub_822AF198, through
// sub_822AF2B0 from sub_822B0840, fit frusta to something else, probably shadows: none should fall in 1.70-1.86).
void GHAerosmithFrustumHook(PPCRegister& f1, PPCRegister& f2, PPCRegister& f3, PPCRegister& f4) {
  double factor = ghaerosmith::g_aspect_factor.load(std::memory_order_relaxed);
  double height = std::fabs(f4.f64 - f3.f64);
  if (factor == 1.0 || height == 0.0) {
    return;
  }
  double aspect = std::fabs(f2.f64 - f1.f64) / height;
  if (aspect < 1.70 || aspect > 1.86) {
    return;
  }
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[ultrawide] 3D views widened ({:.4f})", factor);
  }
  f1.f64 *= factor;
  f2.f64 *= factor;
}
