// ghaerosmith - the launcher's Mods page: the note highway's scale, length, perspective, style and opacity, the whammy
// bar's sound and the instrument's volume on a miss, as GH5's and GH Warriors of Rock's (their src/mods.cpp). The
// highway's are changes of the game's own script data (src/script_globals.cpp), made when the game starts; the sound's
// are hooks on two of its C functions.
//
// - The highway (scripts\guitar\highway_2d.qb's generate_pos_table, at each song's setup): it copies the one-player
//   values (guitar_tweaks.qb's highway_height1 350, highway_top_width1 160, widthOffsetFactor1 2.2, highway_playline1
//   655, gem_start_scale1 0.25, fretbar_start_scale1 0.15, whammy_top_width1 10, whammy_width_offset1 1.8,
//   sidebar_*1, nowbar_*1, string_*1, gHighwayTiling1 1.5), or the two-player ones (*2: 270, 170, 1.33, 1.5), into the
//   globals the highway reads. It is drawn in 2D from y = playline - height to the strike line, highway_top_width wide
//   at the top and top x (1 + widthoffsetfactor) at the bottom (512 for one player), cut into highway_lines = 1152 rows
//   of which real_highway_lines = 1024 reach the strike line, rows linear in time (a note crosses them in
//   scroll_time - destroy_time) whose heights follow h[0] = 1, h[i] = (h[i-1] x f) ^ e, normalised over the 1024. GH5's
//   GeneratePosTable is this script made a C function, with the same geometry to the digit; here f and e come from
//   heightPerspFactTable / heightPerspExpTable at the index highway_height - 162 (0-510): 1.0008860 and 1.0015410 for
//   350 (near rows 9.07 times the far ones; GH5's 1.00108, 1.00125: 9.4), 1.0006759 and 1.0013311 for 270.
//   - Highway perspective p: the projection's compression A = e ln f / (e - 1) times p (the same e): flatter below 100 %,
//     steeper above; the top width a camera gives with that compression (top x sqrt(R) ^ (1 - p), R the near rows over
//     the far ones), the far end's sizes with it and the bottom's unchanged.
//   - Highway length k: k times the time on the highway, the same projection continued beyond the console's far end
//     (e' = e^k, A' = A e^(1024 (1 - k))), so the console's part stays where it was and the notes keep their speed at
//     the strike line; the height then what the longer highway takes, the top width on the console's side lines, the
//     texture's tiling times k, and every difficulty's scroll_time and destroy_time times k (setup_highway's
//     Set2DHighwaySpeed and gem_scroller's time_offset use their difference).
//   - The table entries at the new heights' indexes get the new f and e (each player count's own).
//   - With ghaerosmith_highway_keep_speed and a perspective other than 100 %, the times are also multiplied by the
//     strike line row's share of the highway against the console's: the notes reach the strike line at its speed.
//   - A held sustain's end (whammy_cutoff, guitar_tweaks.qb 1100): while the note is held (the sustain's "holding"
//     tag, CheckNoteHoldStart to CheckNoteHoldEnd), UpdateGemMovers (sub_821154A0) keeps the sustain's near end at the
//     row whammy_cutoff x 1024 / 1152 (977.8, 47 pixels above the strike line on the console, inside the fret's flame),
//     where it is cut; at the hold's end the piece left becomes a dead sustain and slides away. That row is a share of
//     the time on the highway, so a longer or steeper highway drew it higher (length 150 %: 68 pixels; perspective
//     150 %: 70) and left a longer dead piece below the frets. With the length or the perspective changed,
//     whammy_cutoff is the row at the console's distance above the strike line on the one-player highway (length
//     150 %: 1117; the two-player highway's within a few rows); the scale keeps it (every size with it).
//   - Highway scale s (one player): every one-player size times s, anchored at the strike line (the height, the widths,
//     the gems', beat lines', sustains', sidebars', strikers' and Star Power effect's scales, the fade), the projection
//     kept (its table entry at the scaled height's index). GH III has no highway_scale_chart (GH5's): the rock meter and
//     the Star Power meter keep their places.
// - Highway style: highway_normal (guitar_tweaks.qb, [255 255 255 255]) is the highway's color, the texture's
//   multiplier: setup_highway creates the highway's sprite (Highway_2D<p>) with rgba = $highway_normal,
//   GuitarEvent_StarPowerOff sets it back, and a miss's highway_pulse_black morphs it to $highway_pulse ([0 0 0 255])
//   and back. Dark is the community's black highway: [0 0 0 255]. (This game's highway_starpower is read by no script.)
// - Highway opacity: the alpha of highway_normal and highway_pulse, and the cut-out of the venue under the highway left
//   out. create_highway_prepass adds a prepass viewport to bg_viewport with a highway-shaped object seen by its own
//   camera, and if $highwayvisible = ON calls enable_highway_prepass (the viewport's prepass active: the venue is not
//   drawn under the highway, black behind a see-through one). With the opacity below 100, create_highway_prepass reads
//   highwayvisible as OFF: the prepass is never turned on and the venue shows through. (GH5 cleared the prepass flag
//   of the window instead: its create_highway_prepass makes a window element.)
// - Whammy sound effect: control_whammy_pitchshift (scripts\guitar\guitar_input.qb, run while a sustain is held)
//   reads the bar and calls set_whammy_pitchshift (guitar_song.qb), which bends the player's guitar bus with
//   setsoundbusseffects effect = {effect = PitchShift name = Guitar1PitchShift (Guitar2PitchShift for player 2) pitch =
//   (1 - control x 0.057)}; GH5 bends it in C++ (sub_821E8DE0). Off: SetSoundBussEffects (sub_82280420; a C function:
//   r3 its parameters, r4 the calling script, its name at +216) does nothing when set_whammy_pitchshift calls it, so the
//   bus keeps its pitch. The bar still wobbles the sustain on the highway and fills Star Power (SetNewWhammyValue, after
//   it). Every player's, a remote online one's too (net_whammy_pitch_shift runs the same script).
// - Instrument volume on a miss: GuitarEvent_MissedNote (unless player_status.guitar_volume is already 0) and
//   GuitarEvent_UnnecessaryNote (guitar_events.qb) set player_status.guitar_volume = 0 and call UpdateGuitarVolume
//   (sub_82115F50); a hit sets it back to 100 (GuitarEvent_HitNotes_CFunc, sub_82116700). UpdateGuitarVolume reads both
//   players' guitar_volume (r26, r19 at 0x82116168; the last values it set, kept at 0x829B8D84 and +4, in r31 and r24)
//   and sets the guitar_player<n>_unique_id streams' volume to that percentage (with one player, or two on different
//   parts), ramped down in guitarvolumeramptimedown (0.02 s) or up from 0 in guitarvolumeramptimeup (0 s). Here the
//   value that call reads is the percentage, when the calling script is a miss's (r4 at its entry) and the player's last
//   value was not 0, and again in the calls after it until the player's guitar_volume is no longer 0; the game's own
//   mutes keep 0 (a sustain let go early, CheckNoteHoldPerFrame sub_82120280; the start before the first note). The
//   game's guitar_volume stays 0 (the miss scripts and the network messages read it). Two players on the same part (a
//   crossfade of the guitar and its partial stereo level, r18 = 1) and guitar_always_volume_100 (r25 = 1): not changed.
//   From the percentage back to 100 the stream ramps in the down time (0.02 s; the game's up time is 0).

#include "mods.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include "highway_images.h"
#include "script_globals.h"

REXCVAR_DEFINE_INT32(ghaerosmith_highway_scale, 100, "GHAerosmith",
                     "The note highway's size with one player, in percent (80-120 in the launcher; it shrinks and grows "
                     "from the strike line); 100 = as on the console");
REXCVAR_DEFINE_INT32(ghaerosmith_highway_length, 100, "GHAerosmith",
                     "The note highway's length in time, in percent (100, 125, 150 or 170): the notes seen that much "
                     "earlier, the console's highway continued in its own perspective, at the same speed at the strike "
                     "line; 100 = as on the console");
REXCVAR_DEFINE_INT32(ghaerosmith_highway_perspective, 100, "GHAerosmith",
                     "The note highway's perspective, in percent of the console's (25-150 in the launcher): below 100 "
                     "flatter, as seen from higher, above 100 steeper; 100 = as on the console");
REXCVAR_DEFINE_BOOL(ghaerosmith_highway_keep_speed, false, "GHAerosmith",
                    "With a highway perspective other than 100, the notes reach the strike line at the console's speed: "
                    "the times on the highway multiplied by the perspective's speed there (shorter below 100, longer "
                    "above); false = the console's times, the notes slower near the strike line below 100 and faster "
                    "above");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_style, "original", "GHAerosmith",
                      "The note highway's look: original, or dark (the community's black highway)");
REXCVAR_DEFINE_INT32(ghaerosmith_highway_opacity, 100, "GHAerosmith",
                     "The note highway's opacity in percent: below 100 the venue shows through it (the game's cut-out of "
                     "the venue under the highway left out); 100 = as on the console");
REXCVAR_DEFINE_BOOL(ghaerosmith_whammy_sound, true, "GHAerosmith",
                    "The whammy bar bends a held sustain's pitch; false keeps the pitch (the bar still wobbles the note "
                    "on the highway and fills Star Power)");
REXCVAR_DEFINE_BOOL(ghaerosmith_miss_lower, true, "GHAerosmith",
                    "Your instrument's track lowered on a miss or a strum without a note, to ghaerosmith_miss_volume; "
                    "false: it plays on at full volume");
REXCVAR_DEFINE_INT32(ghaerosmith_miss_volume, 0, "GHAerosmith",
                     "How much of your instrument's track stays when you miss a note or strum without one, in percent "
                     "(0-100); 0 = muted as on the console");

namespace ghaerosmith::mods {
namespace {

constexpr double kPlayline = 655.0;  // highway_playline1 and 2
constexpr int kRows = 1024;          // real_highway_lines
constexpr int kAllRows = 1152;       // highway_lines: the rows' tables, to below the strike line
constexpr double kConsoleCutoff = 1100.0;  // whammy_cutoff
constexpr double kHighestTop = 50.0;  // the far end no higher than this (of 720)
constexpr int kTableBase = 162;       // generate_pos_table: the tables' index is highway_height - 162, 0 to 510
constexpr int kTableLast = 510;

double AsFloat(double v) { return double(float(v)); }

struct Projection {
  double f;  // heightperspfact
  double e;  // heightperspexp
};

// One player count's highway as guitar_tweaks.qb has it, and its projection (heightPerspFactTable /
// heightPerspExpTable at its height's index).
struct Console {
  const char* suffix;  // "1" or "2"
  double height;
  double top_width;
  double width_offset;         // widthOffsetFactor
  double whammy_width_offset;  // whammy_width_offset
  Projection projection;
  double BottomWidth() const { return top_width * (1.0 + width_offset); }
};

const Console kOnePlayer{"1", 350.0, 160.0, AsFloat(2.2), AsFloat(1.8),
                         {AsFloat(1.0008859634399414), AsFloat(1.001541018486023)}};
const Console kTwoPlayers{"2", 270.0, 170.0, AsFloat(1.33), AsFloat(1.5),
                          {AsFloat(1.000675916671753), AsFloat(1.001331090927124)}};

double Exponent(const Projection& p) { return p.e * std::log(p.f) / (p.e - 1.0); }

// The projection with its compression times `perspective` and its time window times `length`.
Projection Transform(const Projection& in, double perspective, double length) {
  const double a = Exponent(in) * perspective * std::pow(in.e, kRows * (1.0 - length));
  const double e = std::pow(in.e, length);
  return {std::exp(a * (e - 1.0) / e), e};
}

// The place from the far end (0) to the strike line (1) of a fractional row, as generate_pos_table makes them.
double Place(const Projection& p, double row) {
  std::vector<double> h(kRows + 1);
  h[0] = 1.0;
  for (int i = 1; i <= kRows; ++i) {
    h[i] = std::pow(h[i - 1] * p.f, p.e);
  }
  double sum = 0.0;
  for (int i = 0; i < kRows; ++i) {
    sum += h[i];
  }
  const int whole = std::clamp(int(std::floor(row)), 0, kRows - 1);
  double place = 0.0;
  for (int i = 0; i < whole; ++i) {
    place += h[i];
  }
  place += h[whole] * (row - whole);
  return place / sum;
}

// The notes' speed in pixels at the strike line with the perspective p, against the console's: the last row's share of
// the highway (the same for every length: the length continues the projection beyond the far end).
double StrikeLineSpeed(const Console& console, double p) {
  const auto last_row = [](const Projection& projection) { return 1.0 - Place(projection, kRows - 1); };
  return last_row(Transform(console.projection, p, 1.0)) / last_row(console.projection);
}

struct Shape {
  Projection projection;
  double height;
  double top_width;
};

// A player count's highway for a perspective p and a length k (1, 1 = the console's).
Shape Make(const Console& console, double p, double k) {
  Shape out;
  out.projection = Transform(console.projection, p, k);
  // The console's far end, at the time T, is the new row 1024 (1 - 1/k); it stays where it was.
  out.height = k == 1.0 ? console.height
                        : console.height / (1.0 - Place(out.projection, kRows * (1.0 - 1.0 / k)));
  // The top width a camera gives with this compression, at the console's far end (the width goes with the square root of
  // the rows' heights); beyond it on the same side lines.
  const double bottom = console.BottomWidth();
  const double sqrt_r =
      std::exp(Exponent(console.projection) * (std::pow(console.projection.e, kRows) - 1.0) / 2.0);
  const double top_at_far_end = std::min(bottom, console.top_width * std::pow(sqrt_r, 1.0 - p));
  out.top_width = top_at_far_end + (bottom - top_at_far_end) * (console.height - out.height) / console.height;
  return out;
}

// The rows' places as UpdateGemMovers reads them for a fractional row r: time_accum_table[r] (the rows' heights,
// normalised over the 1024, summed through r) plus the fraction times gem_time_table512[r + 1] - gem_time_table512[r].
class Places {
 public:
  explicit Places(const Projection& p) : step_(kAllRows), through_(kAllRows) {
    std::vector<double> h(kAllRows);
    h[0] = 1.0;
    for (int i = 1; i < kAllRows; ++i) {
      h[i] = std::pow(h[i - 1] * p.f, p.e);
    }
    double sum = 0.0;
    for (int i = 0; i < kRows; ++i) {
      sum += h[i];
    }
    for (int i = 0; i < kAllRows; ++i) {
      step_[i] = h[i] / sum;
      through_[i] = (i ? through_[i - 1] : 0.0) + step_[i];
    }
  }
  double At(double row) const {
    const int whole = std::clamp(int(row), 0, kAllRows - 1);
    const int next = std::min(whole + 1, kAllRows - 1);
    return through_[whole] + (step_[next] - step_[whole]) * (row - whole);
  }

 private:
  std::vector<double> step_;
  std::vector<double> through_;
};

// whammy_cutoff for a highway shape: the row where a held sustain is cut at the console's distance above the strike line.
double WhammyCutoff(const Console& console, const Shape& shape) {
  const Places before(console.projection);
  const Places after(shape.projection);
  const auto above = [](const Places& places, double height, double row) {
    return (places.At(row) - places.At(kRows)) * height;  // negative: pixels above the strike line
  };
  const double target = above(before, console.height, kConsoleCutoff * kRows / kAllRows);
  double low = 0.0;
  double high = kRows;
  for (int i = 0; i < 50; ++i) {
    const double middle = (low + high) / 2.0;
    (above(after, shape.height, middle) < target ? low : high) = middle;
  }
  return low * kAllRows / kRows;
}

// The changes of one player count's highway: the shape `shape` at the scale `s` (one player only).
void Apply(const Console& c, const Shape& shape, double s, bool changed_shape, double extra_height) {
  const std::string n = c.suffix;
  const double height = std::round(shape.height * s) + extra_height;
  const double width_ratio = shape.top_width / c.top_width;  // the far end's sizes
  if (changed_shape || s != 1.0) {
    script::Set("highway_height" + n, height);
    script::Set("highway_top_width" + n, shape.top_width * s);
  }
  if (changed_shape) {
    script::Set("widthOffsetFactor" + n, c.BottomWidth() / shape.top_width - 1.0);
    // The sustain's width at the bottom kept: its top times the far end's ratio, its offset to the same bottom.
    script::Set("whammy_width_offset" + n, (1.0 + c.whammy_width_offset) / width_ratio - 1.0);
  }
  const double far = width_ratio * s;
  if (far != 1.0) {
    script::Scale("gem_start_scale" + n, far);
    script::Scale("fretbar_start_scale" + n, far);
    script::Scale("whammy_top_width" + n, far);
  }
  // The sidebars lie along the slanted sides: their length with the side's.
  const auto side = [](double h, double top, double bottom) { return std::hypot(h, (bottom - top) / 2.0); };
  const double sidebar = side(shape.height, shape.top_width, c.BottomWidth()) /
                         side(c.height, c.top_width, c.BottomWidth()) * s;
  if (sidebar != 1.0) {
    script::Scale("sidebar_y_scale" + n, sidebar);
  }
  if (s != 1.0) {
    for (const char* name : {"gem_star_scale", "sidebar_x_offset", "sidebar_x_scale", "starpower_fx_scale",
                             "nowbar_scale_x", "nowbar_scale_y", "string_scale_x", "string_scale_y", "highway_fade"}) {
      script::Scale(name + n, s);
    }
  }
  // The projection at the index generate_pos_table will read (the two player counts' heights differ: 350 x s or
  // more against 270 or more, s at least 0.8).
  const int index = std::clamp(int(height) - kTableBase, 0, kTableLast);
  script::Set("heightPerspFactTable[" + std::to_string(index) + "]", shape.projection.f);
  script::Set("heightPerspExpTable[" + std::to_string(index) + "]", shape.projection.e);
}

}  // namespace

void Configure() {
  static bool configured = false;
  if (configured) {
    return;
  }
  configured = true;
  std::string summary;

  // The highway's geometry.
  const int32_t scale = std::clamp(REXCVAR_GET(ghaerosmith_highway_scale), 50, 150);
  const int32_t perspective = std::clamp(REXCVAR_GET(ghaerosmith_highway_perspective), 10, 200);
  const int32_t asked = std::clamp(REXCVAR_GET(ghaerosmith_highway_length), 100, 300);
  // The length that fits: the far end, with the one-player scale, no higher than y = 50.
  int32_t length = asked;
  const double room = (kPlayline - kHighestTop) / std::max(1.0, scale / 100.0);
  while (length > 100 && Make(kOnePlayer, perspective / 100.0, length / 100.0).height > room) {
    length -= 5;
  }
  if (length != asked) {
    REXLOG_INFO("[mods] highway length {} % with perspective {} % and scale {} %: {} %, the most that fits", asked,
                perspective, scale, length);
  }
  const double s = scale / 100.0;
  const double p = perspective / 100.0;
  const double k = length / 100.0;
  const bool changed_shape = perspective != 100 || length != 100;
  if (changed_shape || scale != 100) {
    const Shape one = Make(kOnePlayer, p, k);
    const Shape two = changed_shape ? Make(kTwoPlayers, p, k) : Shape{kTwoPlayers.projection, kTwoPlayers.height,
                                                                        kTwoPlayers.top_width};
    if (std::round(one.height * s) == std::round(two.height)) {
      // Both would read the same table entry: the one-player highway a pixel taller.
      REXLOG_WARN("[mods] the one- and two-player highways at the same height ({:.0f}): one player's one higher",
                  std::round(two.height));
    }
    Apply(kOnePlayer, one, s, changed_shape, std::round(one.height * s) == std::round(two.height) ? 1.0 : 0.0);
    if (changed_shape) {
      Apply(kTwoPlayers, two, 1.0, true, 0.0);
    }
    if (k != 1.0) {
      script::Scale("gHighwayTiling1", k);
      script::Scale("gHighwayTiling2", k);
    }
    if (changed_shape) {
      // A held sustain cut where it is on the console, inside the fret's flame.
      const double cutoff = WhammyCutoff(kOnePlayer, one);
      script::Set("whammy_cutoff", cutoff);
      REXLOG_INFO("[mods] a held sustain cut at whammy_cutoff {:.1f} (the console's 1100: {:.0f} pixels above the strike "
                  "line)",
                  cutoff, -(Places(kOnePlayer.projection).At(kConsoleCutoff * kRows / kAllRows) -
                            Places(kOnePlayer.projection).At(kRows)) * kOnePlayer.height);
    }
    // The times on the highway: the length's, and with ghaerosmith_highway_keep_speed the perspective's speed.
    const double speed = perspective != 100 ? StrikeLineSpeed(kOnePlayer, p) : 1.0;
    const bool keep_speed = perspective != 100 && REXCVAR_GET(ghaerosmith_highway_keep_speed);
    const double time = k * (keep_speed ? speed : 1.0);
    if (time != 1.0) {
      script::Scale("difficulty_list_props.*.scroll_time", time);
      script::Scale("destroy_time", time);
    }
    if (scale != 100) {
      summary += " highway scale " + std::to_string(scale) + " %";
    }
    if (perspective != 100) {
      summary += " highway perspective " + std::to_string(perspective) + " %";
      REXLOG_INFO("[mods] highway perspective {} %: the notes reach the strike line at {:.2f} of the console's speed{}",
                  perspective, keep_speed ? 1.0 : speed,
                  keep_speed ? " (ghaerosmith_highway_keep_speed: the times on the highway x " +
                                   std::to_string(speed) + ")"
                             : "");
    }
    if (length != 100) {
      summary += " highway length " + std::to_string(length) + " %";
    }
    REXLOG_INFO("[mods] the one-player highway: {:.0f} high (far end at y = {:.0f}), {:.1f} wide at the top, "
                "perspective {:.7f} {:.7f}",
                std::round(one.height * s), kPlayline - std::round(one.height * s), one.top_width * s,
                one.projection.f, one.projection.e);
  }

  const std::string style = REXCVAR_GET(ghaerosmith_highway_style);
  if (style == "dark") {
    for (int i = 0; i < 3; ++i) {
      script::Set("highway_normal[" + std::to_string(i) + "]", 0);
    }
    summary += " highway style dark";
  } else if (style != "original" && !style.empty()) {
    REXLOG_WARN("[mods] ghaerosmith_highway_style \"{}\": not a style (original, dark); the original", style);
  }

  const int32_t opacity = std::clamp(REXCVAR_GET(ghaerosmith_highway_opacity), 0, 100);
  if (opacity < 100) {
    const double alpha = std::round(255.0 * opacity / 100.0);
    script::Set("highway_normal[3]", alpha);
    script::Set("highway_pulse[3]", alpha);
    script::ReadAsName("create_highway_prepass", "highwayvisible", "off");
    summary += " highway opacity " + std::to_string(opacity) + " %";
  }

  if (!REXCVAR_GET(ghaerosmith_whammy_sound)) {
    summary += " whammy sound off";
  }
  const int32_t miss = MissVolume();
  if (miss > 0) {
    summary += " instrument on a miss " + std::to_string(miss) + " %";
  }

  if (!summary.empty()) {
    REXLOG_INFO("[mods]{}", summary);
  }
  highway::Configure();  // the highway's picture (src/highway_images.cpp)
}

int32_t MissVolume() {
  static const int32_t volume =
      REXCVAR_GET(ghaerosmith_miss_lower) ? std::clamp(REXCVAR_GET(ghaerosmith_miss_volume), 0, 100) : 100;
  return volume;
}

}  // namespace ghaerosmith::mods

namespace {

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

constexpr uint32_t kScriptName = 216;                  // the running script's name in the interpreter's script object
constexpr uint32_t kSetWhammyPitchshift = 0xDCFE15A0;  // set_whammy_pitchshift
constexpr uint32_t kMissedNote = 0x8D14D919;           // GuitarEvent_MissedNote
constexpr uint32_t kUnnecessaryNote = 0x1D649F39;      // GuitarEvent_UnnecessaryNote

// The script that called UpdateGuitarVolume (0: called from the game's code with no script).
thread_local uint32_t t_volume_caller = 0;
// Per player: the last UpdateGuitarVolume set the miss's percentage (until its guitar_volume is no longer 0).
std::atomic<bool> g_kept[2] = {false, false};
std::atomic<int> g_whammy_logged{0};
std::atomic<int> g_miss_logged{0};

uint32_t ScriptName(const uint8_t* base, uint32_t script) { return script ? Load32(base, script + kScriptName) : 0; }

// One player's volume as UpdateGuitarVolume read it (`volume`, the last value it set `last`): the miss's percentage
// instead of 0 after a miss.
void KeepOnMiss(int player, PPCRegister& volume, uint32_t last, bool miss, int32_t percent) {
  if (volume.u32 != 0) {
    g_kept[player].store(false);
    return;
  }
  if (!g_kept[player].load() && !(miss && last != 0)) {
    return;  // the game's own mute
  }
  volume.u64 = uint64_t(percent);
  if (!g_kept[player].exchange(true) && g_miss_logged.fetch_add(1) < 4) {
    REXLOG_INFO("[mods] a miss (script {:08X}, player {}): the instrument kept at {} %", t_volume_caller, player + 1,
                percent);
  }
}

}  // namespace

// [[midasm_hook]] 0x82280420 (the entry of SetSoundBussEffects, sub_82280420), return_on_true: r3 = its parameters, r4 =
// the calling script. With ghaerosmith_whammy_sound off, set_whammy_pitchshift's bend of the guitar bus is not applied.
bool GHAerosmithWhammySoundHook(PPCRegister& r3, PPCRegister& r4) {
  static const bool on = REXCVAR_GET(ghaerosmith_whammy_sound);
  if (on) {
    return false;
  }
  const uint8_t* base = GuestBase();
  if (!base || ScriptName(base, r4.u32) != kSetWhammyPitchshift) {
    return false;
  }
  if (g_whammy_logged.fetch_add(1) < 2) {
    REXLOG_INFO("[mods] set_whammy_pitchshift's bend not applied (ghaerosmith_whammy_sound off)");
  }
  r3.u64 = 1;
  return true;
}

// [[midasm_hook]] 0x82115F50 (the entry of UpdateGuitarVolume, sub_82115F50): r4 = the calling script.
void GHAerosmithGuitarVolumeHook(PPCRegister& r4) {
  if (ghaerosmith::mods::MissVolume() <= 0) {
    return;
  }
  const uint8_t* base = GuestBase();
  t_volume_caller = base ? ScriptName(base, r4.u32) : 0;
}

// [[midasm_hook]] 0x82116168 (UpdateGuitarVolume, once both players' values are read; a branch target): r26 / r19 =
// player 1's / 2's guitar_volume, r31 / r24 = the last values it set, r18 = 1 with two players on the same part, r25 =
// 1 with guitar_always_volume_100.
void GHAerosmithMissVolumeHook(PPCRegister& r18, PPCRegister& r19, PPCRegister& r24, PPCRegister& r25,
                               PPCRegister& r26, PPCRegister& r31) {
  const int32_t percent = ghaerosmith::mods::MissVolume();
  if (percent <= 0) {
    return;
  }
  if (r18.u32 == 1 || r25.u32 == 1) {
    g_kept[0].store(false);
    g_kept[1].store(false);
    return;
  }
  const bool miss = t_volume_caller == kMissedNote || t_volume_caller == kUnnecessaryNote;
  KeepOnMiss(0, r26, r31.u32, miss, percent);
  KeepOnMiss(1, r19, r24.u32, miss, percent);
}
