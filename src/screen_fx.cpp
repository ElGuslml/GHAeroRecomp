// ghaerosmith - the game's screen effects that can be turned off: the bloom and the depth of field (as GH5's
// src/screen_fx.cpp offers them).
//
// Guitar Hero III's engine makes its screen effects with the scripts' ScreenFX_AddFXInstance (sub_82274418): the
// viewport's effects manager (its virtual +68) adds one by its type, sub_82381958 (r3 the manager, r4 the parameters,
// r5 the script), whose "type" is one of fog, bloom, poisson_dof, noise, dof, volume_fog, fisheye, blur, simple_bloom,
// bright_sat, vignette and nearblur, each its own class. ScreenFX_UpdateFXInstanceParams (sub_82380EB0) changes one by
// its name and does nothing when there is none. GH Aerosmith's scripts use few of them (scripts\engine\tod_profiles.qb;
// the venues' packs define none of their own):
// - Default_TOD_Manager, every venue's look (TOD_Proxim_Update_LightFX_Viewport): bright_sat (neutral: contrast,
//   brightness, saturation and hue 1, the color mixes the identity) and simple_bloom (bloom1 0.35): the glow.
// - DOF_* (CameraCuts_ApplyDOF, venue_DOF on bg_viewport): the depth of field of the songs' camera shots; the guitar
//   selection's GSdof and GSdof2 (guitar_select_camera_morph).
// - ScreenToBlack_TOD_Manager's vignette is the fade to black, ScreenFlash_TOD_Manager's bright_sat the flashes, and
//   TOD_Proxim_Update_Global_Brightness's bright_sat the brightness option: none is a switch here.
// An effect turned off is not made: the hook at sub_82381958's entry finds the parameters' type as the game's
// sub_82146B90 does (a component named "type", or inside an unnamed structure: an expression's, a local's or a
// global's) and returns true without adding it.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <atomic>
#include <cstdint>
#include <cstring>

REXCVAR_DEFINE_BOOL(ghaerosmith_fx_bloom, true, "GHAerosmith",
                    "Screen effects: the bloom, the glow around lights and bright surfaces; false = off");
REXCVAR_DEFINE_BOOL(ghaerosmith_fx_depth_of_field, true, "GHAerosmith",
                    "Screen effects: the depth of field, the background blurred in some camera shots and in the guitar "
                    "selection; false = off");

namespace {

constexpr uint32_t kType = 0x7321A8D6;  // "type"
constexpr uint32_t kBloom = 0x393282E4;
constexpr uint32_t kSimpleBloom = 0x8B2068AE;
constexpr uint32_t kDof = 0x09D4F614;
constexpr uint32_t kPoissonDof = 0x008D9389;
constexpr uint32_t kNearBlur = 0xFA290E62;
constexpr uint32_t kGlobals = 0x82B94030;  // the script globals' table (src/script_globals.cpp)

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

// A script global's node (+2 its type, +4 its name, +12 its value, +16 the next of its bucket), or 0.
uint32_t FindGlobal(const uint8_t* base, uint32_t name) {
  const uint32_t table = Load32(base, kGlobals);
  if (!table) return 0;
  for (uint32_t node = Load32(base, table + ((name & 0x7FFF) << 2)); node; node = Load32(base, node + 16)) {
    if (Load32(base, node + 4) == name) return node;
  }
  return 0;
}

// The checksum named `key` in the structure at `structure` (its first component at +4; a component's type in the low
// 7 bits of +1, its name at +4, its value at +8, the next at +12), searched in its unnamed structures too.
uint32_t FindName(const uint8_t* base, uint32_t structure, uint32_t key, int depth) {
  if (!structure || depth > 4) return 0;
  int count = 0;
  for (uint32_t c = Load32(base, structure + 4); c && count < 256; c = Load32(base, c + 12), ++count) {
    const uint8_t type = base[c + 1] & 0x7F;
    const uint32_t name = Load32(base, c + 4);
    const uint32_t value = Load32(base, c + 8);
    if (name == key) {
      if (type == 13) return value;
      if (type == 26) {
        const uint32_t node = FindGlobal(base, value);
        if (node && base[node + 2] == 13) return Load32(base, node + 12);
      }
      return 0;
    }
    if (name != 0) continue;
    if (type == 22 || type == 23) {
      if (const uint32_t found = FindName(base, value, key, depth + 1)) return found;
    } else if (type == 26) {
      const uint32_t node = FindGlobal(base, value);
      if (node && base[node + 2] == 10) {
        if (const uint32_t found = FindName(base, Load32(base, node + 12), key, depth + 1)) return found;
      }
    }
  }
  return 0;
}

}  // namespace

// [[midasm_hook]] 0x82381958 (the entry of sub_82381958, a screen effect added to a viewport's manager),
// return_on_true: r4 = its parameters. An effect turned off is not made (r3 = 1, as when it is).
bool GHAerosmithScreenFxAddHook(PPCRegister& r3, PPCRegister& r4) {
  static const bool bloom = REXCVAR_GET(ghaerosmith_fx_bloom);
  static const bool dof = REXCVAR_GET(ghaerosmith_fx_depth_of_field);
  if (bloom && dof) {
    return false;
  }
  const uint8_t* base = GuestBase();
  if (!base || !r4.u32) {
    return false;
  }
  const uint32_t type = FindName(base, r4.u32, kType, 0);
  const bool off = (!bloom && (type == kBloom || type == kSimpleBloom)) ||
                   (!dof && (type == kDof || type == kPoissonDof || type == kNearBlur));
  if (!off) {
    return false;
  }
  static std::atomic<int> logged{0};
  if (logged.fetch_add(1) < 4) {
    REXLOG_INFO("[screen fx] a {} effect not made (ghaerosmith_fx_{})",
                type == kBloom || type == kSimpleBloom ? "bloom" : "depth of field",
                type == kBloom || type == kSimpleBloom ? "bloom" : "depth_of_field");
  }
  r3.u64 = 1;
  return true;
}
