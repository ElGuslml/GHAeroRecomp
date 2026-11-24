// ghaerosmith - frame rates above 60: the emulated vertical blank at the chosen rate.
//
// As GH Metallica (GHMetalRecomp's src/framerate.cpp, docs/frame-rate.md): the game's pacing is all vertical blanks
// (the render thread presenting a frame in sub_822CE808 and the main thread's end of frame in sub_822C36A0 wait on the
// vertical blank counters, src/thread_waits.cpp), and with the SDK's emulated vblank at a higher rate
// (video_mode_refresh_rate, which src/options.cpp sets from ghaerosmith_frame_rate) it presents at that rate: 120 FPS
// in a song at 120 Hz.
//
// What the game counts in frames needs a hand above 60:
// - Script waits in game frames ("Wait N gameframe(s)", 249 of the game's scripts): N frames are N / 60 s on the
//   console. Held to that (below), except the scripts that run the game's per-frame logic, which would lose inputs or
//   time at 60 passes a second.
// - Button presses polled by scripts (ControllerMake) last one frame: a script whose waits are held is given the
//   presses of the frames it skipped.
// - The frame's time a held script asks for (GetDeltaTime): the time since its previous pass (the highway rising at a
//   song's start).
// - The whammy on a sustain: in this engine a script, star_power_whammy (below).
// - Havok's world, stepped by 1/60 s each frame while the bodies that follow the animation move by the frame's time
//   (the hair and the guitar cables).
// - The loading screen's wheel and wave, turned and moved by a step each frame.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <utility>

REXCVAR_DEFINE_INT32(ghaerosmith_frame_rate, 60, "GHAerosmith",
                     "Frame rate: 60 as on the console, or 120, 144, 160, 165 or 240: the emulated vertical blank runs "
                     "at that rate (video_mode_refresh_rate, set at startup) and the game presents on every one; its "
                     "clock is real time, so it keeps its speed");

REXCVAR_DEFINE_BOOL(ghaerosmith_frame_rate_fix, true, "GHAerosmith",
                    "With the vertical blank above 60 Hz: what the game counts in frames keeps the console's speed "
                    "(script waits in game frames, the whammy on sustains) and scripts are given the button presses "
                    "of the frames they skip (false = those run faster, to compare)");

namespace {

// The emulated vertical blanks per second (the SDK's vblank thread): video_mode_refresh_rate with vsync, one a
// millisecond without. Read once a second.
double VblankRate() {
  thread_local double rate = 60.0;
  thread_local std::chrono::steady_clock::time_point read;
  const auto now = std::chrono::steady_clock::now();
  if (read == std::chrono::steady_clock::time_point{} || now - read > std::chrono::seconds(1)) {
    read = now;
    rate = rex::cvar::Query<bool>("vsync") ? std::clamp(rex::cvar::Query<double>("video_mode_refresh_rate"), 24.0, 240.0)
                                           : 1000.0;
  }
  return rate;
}

bool HighRate() { return REXCVAR_GET(ghaerosmith_frame_rate_fix) && VblankRate() != 60.0; }

double FrameSeconds() { return 1.0 / VblankRate(); }

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
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

// Script waits in game frames: the script function Wait (sub_8225FD10) sets a type 1 wait through sub_82323548 for
// "gameframe(s)" (or a type 8 through sub_823234F8), its count at +4 of the wait (the script + 168); sub_82323888, each
// frame, takes one from the count and ends the wait in the frame after it reaches 0, so "Wait N gameframes" lasts N
// frames: N / 60 s on the console, half that at 120 FPS. Above 60 Hz the count is held above 0 and let reach 0 in the
// frame before the one nearest the wait's end at 60 frames a second, as GH Metallica's (a wait set right as the previous
// one on the same script ended begins at that one's end, so a loop keeps the console's pace on average).
//
// Unlike GH Metallica, this engine runs much of its per-frame logic from scripts that call a C function and wait one
// game frame, in a loop: the guitar's input (GuitarInputLogicPerFrame), the button checker, star power's activation
// (check_for_star_power_CFunc), the note holds, the score, the gem, fretbar, fret finger and drum iterators, the
// scripts that follow the song's time (GetSongTime(Ms)), the movies (skipped with a button), the menus that poll the
// controllers (GetButtonsPressed, ControllerMake...), and star power's drain, which takes the frame's time
// (current_deltatime) off the meter each pass. Held to 60 passes a second they would see half the frames: presses lost
// (the intro's movies could not be skipped with a short press at 120), star power lasting twice as long (GH Metallica's
// star power at 160 FPS). They keep running every frame: the 67 scripts below, every script that waits in game frames
// and calls a function of the game's named *_CFunc, *PerFrame*, GetSongTime*, GetFrameLength, *Controller*, *Button*,
// *Guitar*, *Movie*, *Input*, UpdateGuitarVolume or LightShow_Update, or reads current_deltatime (found in the game's
// scripts, DATA/COMPRESSED/PAK/qb.pab.xen; the script's name at +48 of its wait, +216 of the script), but four that
// step by the pass and so are held: the lag calibration (0x0F604F14: its circles move 1/60 s a pass, the beat sounds
// when one reaches the line; its strums come through ControllerMake, below), star power by tilting the guitar
// (0xB98F8D67: 30 passes tilted), and two debug ones (0x1D22B902, 0x75EF2E10); and star_power_whammy (below). The other
// 181 (the menus' and the screens' animations, mostly) are held.
namespace {

constexpr uint32_t kPerFrameScripts[] = {
    0x008F66B6, 0x057C9B84, 0x09E5711E, 0x0F2E4B5C, 0x163B8607, 0x18DFBF8B, 0x1FC360DF, 0x2181918C, 0x2223B7E5,
    0x2E30346F, 0x2FBDF9B3, 0x3C3A3173, 0x3C521982, 0x3E6A9EB0, 0x3F070B58, 0x403FAF19, 0x4ABCF817, 0x4C2182B7,
    0x4E850682, 0x5076AABF, 0x5442074B, 0x5C959C3A, 0x60D16CA4, 0x6316C3CE, 0x647F97EE, 0x6524C315, 0x69FA5FAB,
    0x6DF25557, 0x7093BC74, 0x716BADDC, 0x722ACAE8, 0x7442E752, 0x74B936AB, 0x7810765D, 0x78224A69, 0x7AC12BE9,
    0x8381D7A5, 0x868822AA, 0x87369908, 0x8C55B375, 0x94EF431E, 0xABCF4057, 0xAD45AA17, 0xAEDD19EC, 0xAF2C09E2,
    0xB042BC5B, 0xB1330633, 0xB628E45F, 0xBB4D4DD9, 0xC8580B17, 0xC99F0FF6, 0xCB4EFC8F, 0xD1260990, 0xD281E821,
    0xD30450D2, 0xDBA62F8D, 0xE154C2BB, 0xE1FBFB10, 0xE3E24555, 0xE6D4BFFE, 0xEAA85160, 0xEF72FEDF, 0xF0D066B9,
    0xF1C14221, 0xF851828E, 0xFB5889BC, 0xFC6ADC01,
};

// star_power_whammy: the whammy bar on a sustain, every game frame: the bar's position against the previous pass's
// (moving when it moved more than 0.03), a countdown of 5 passes while moving, 5 passes in a row to turn a star power
// sustain blue or back, and star power grown by current_deltatime each pass while moving. Every frame at 120 FPS the bar
// moves half as far between passes and 5 passes are half as long (GH Metallica, whose engine did the same in C++:
// Gustavo saw the blue come and go while he moved the bar). Its waits are held to 60 passes a second, and each pass sees
// current_deltatime as the time since its previous one, so star power grows as fast as at 60.
constexpr uint32_t kStarPowerWhammyScript = 0x8FC2CCAD;

bool PerFrameScript(uint32_t name) {
  return std::find(std::begin(kPerFrameScripts), std::end(kPerFrameScripts), name) != std::end(kPerFrameScripts);
}

uint32_t WaitingScript(const uint8_t* base, uint32_t wait) { return LoadWord(base, wait + 48); }

struct FrameWait {
  std::chrono::steady_clock::time_point start;  // when it would begin at 60 frames a second
  std::chrono::steady_clock::time_point end;    // start + frames / 60 s
};
std::mutex g_frame_waits_mutex;
std::unordered_map<uint32_t, FrameWait> g_frame_waits;  // by the wait's guest address (the script + 168)

// The script global current_deltatime (the frame's time in seconds, set each frame by the engine), in the globals'
// hash table (sub_82145018's: buckets by the key's low 15 bits, entries {+2 type, +4 key, +12 value, +16 next}).
constexpr uint32_t kGlobalsTable = 0x82B94030;
constexpr uint32_t kCurrentDeltaTime = 0x6D735468;

uint32_t GlobalEntry(const uint8_t* base, uint32_t key) {
  const uint32_t table = LoadWord(base, kGlobalsTable);
  if (!table) {
    return 0;
  }
  uint32_t entry = LoadWord(base, table + ((key & 0x7FFF) << 2));
  for (int length = 0; entry && length < 4096 && LoadWord(base, entry + 4) != key; ++length) {
    entry = LoadWord(base, entry + 16);
  }
  return entry && base[entry + 2] == 2 ? entry : 0;  // a float
}

// star_power_whammy's passes: the previous one's start, and current_deltatime while a pass runs with its own.
struct WhammyPasses {
  std::chrono::steady_clock::time_point previous;
  uint32_t entry = 0;        // current_deltatime's entry while replaced, else 0
  uint32_t frame_value = 0;  // the frame's own value, put back at the pass's end
};
std::mutex g_whammy_mutex;
std::unordered_map<uint32_t, WhammyPasses> g_whammy;  // by the wait's guest address

}  // namespace

// [[midasm_hook]] at the entry of sub_82323548 and sub_823234F8 (setting a game-frame wait): r3 = the wait, r4 = N.
void GHAerosmithFrameWaitSetHook(PPCRegister& r3, PPCRegister& r4) {
  uint8_t* base = GuestBase();
  if (!base) {
    return;
  }
  const uint32_t script = WaitingScript(base, r3.u32);
  if (script == kStarPowerWhammyScript) {
    // The end of a pass: current_deltatime back to the frame's.
    std::lock_guard lock(g_whammy_mutex);
    WhammyPasses& passes = g_whammy[r3.u32];
    if (passes.entry) {
      StoreWord(base, passes.entry + 12, passes.frame_value);
      passes.entry = 0;
    }
  }
  if (!HighRate() || PerFrameScript(script)) {
    std::lock_guard lock(g_frame_waits_mutex);
    g_frame_waits.erase(r3.u32);
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const auto frame = std::chrono::duration<double>(FrameSeconds());
  std::lock_guard lock(g_frame_waits_mutex);
  FrameWait& wait = g_frame_waits[r3.u32];
  const bool follows = wait.end != std::chrono::steady_clock::time_point{} &&
                       std::chrono::abs(std::chrono::duration<double>(now - wait.end)) <= 1.5 * frame;
  wait.start = follows ? wait.end : now;
  wait.end = wait.start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                              std::chrono::duration<double>(double(int32_t(r4.u32)) / 60.0));
}

// [[midasm_hook]] 0x823238E0 (sub_82323888, a game-frame wait's count above 0): before "addi r11,r11,-1" and its
// store to +4, r3 = the wait, r11 = the count. Set to 1 (it becomes 0, the wait ends next frame) once the wait's end is
// less than a frame and a half away, so that next frame is the one nearest to it; else to 2 (it stays waiting).
void GHAerosmithFrameWaitCountHook(PPCRegister& r3, PPCRegister& r11) {
  if (!HighRate()) {
    return;
  }
  std::chrono::steady_clock::time_point end;
  {
    std::lock_guard lock(g_frame_waits_mutex);
    auto it = g_frame_waits.find(r3.u32);
    if (it == g_frame_waits.end()) {
      return;
    }
    end = it->second.end;
  }
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[frame rate] script waits in game frames last as at 60 FPS (vblank at {:.0f} Hz)", VblankRate());
  }
  const double left = std::chrono::duration<double>(end - std::chrono::steady_clock::now()).count();
  r11.u64 = left < 1.5 * FrameSeconds() ? 1 : 2;
}

// [[midasm_hook]] 0x823238EC (sub_82323888, a type 1 game-frame wait whose count is 0): before "b sub_823232B0", which
// ends the wait, and the script runs on from it this frame: r3 = the wait. A star_power_whammy pass begins: its
// current_deltatime is the time since its previous pass (at most a tenth of a second) until its next wait.
void GHAerosmithFrameWaitEndHook(PPCRegister& r3) {
  uint8_t* base = GuestBase();
  if (!base || !HighRate() || WaitingScript(base, r3.u32) != kStarPowerWhammyScript) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard lock(g_whammy_mutex);
  WhammyPasses& passes = g_whammy[r3.u32];
  const double elapsed = std::chrono::duration<double>(now - passes.previous).count();
  passes.previous = now;
  const uint32_t entry = GlobalEntry(base, kCurrentDeltaTime);
  if (!entry || passes.entry || elapsed <= 0.0 || elapsed > 0.1) {
    return;
  }
  passes.entry = entry;
  passes.frame_value = LoadWord(base, entry + 12);
  float seconds = float(elapsed);
  uint32_t bits;
  std::memcpy(&bits, &seconds, 4);
  StoreWord(base, entry + 12, bits);
}

// The script function GetDeltaTime (sub_8219A0C8) puts the frame's time in the script's delta_time: the time scaled by
// the slow motion (0x828ED564), or with ignore_slomo the frame's own (sub_82175D40: 0x82AE381C). A held script that
// calls it each pass would see a fraction of a frame per pass and move that much slower: move_highway_2d (0x3979ADFF),
// which raises the highway at a song's start with a speed damped each pass, left it low at 120 FPS and below the screen
// at 160 (Gustavo); the same for a timeline of events (0x79AFEDEC), a tutorial's timer (0x9AC8DAC5) and a blinking
// element (0xA6108217). A script whose game-frame waits are held is given the time since its previous call instead (a
// 60th of a second at its first call or after more than 4 / 60 s, the engine's own longest frame), with the slow
// motion's scale.
namespace {

thread_local uint32_t g_delta_script = 0;  // GetDeltaTime's script, from its entry to its store
std::mutex g_delta_mutex;
std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> g_delta_calls;  // by the script

constexpr uint32_t kFrameTime = 0x82AE381C;  // the frame's time in seconds, without the slow motion (float)

}  // namespace

// [[midasm_hook]] 0x8219A0DC (sub_8219A0C8, GetDeltaTime: "mr r31,r4"): r4 = the script.
void GHAerosmithDeltaTimeScriptHook(PPCRegister& r4) { g_delta_script = r4.u32; }

// [[midasm_hook]] 0x8219A128 (sub_8219A0C8: the call that stores delta_time in the script's locals): f1 = the time.
void GHAerosmithDeltaTimeHook(PPCRegister& f1) {
  const uint32_t script = std::exchange(g_delta_script, 0);
  uint8_t* base = GuestBase();
  if (!script || !base || !HighRate() || PerFrameScript(LoadWord(base, script + 216))) {
    return;
  }
  {
    std::lock_guard lock(g_frame_waits_mutex);
    if (!g_frame_waits.contains(script + 168)) {
      return;  // not held
    }
  }
  const auto now = std::chrono::steady_clock::now();
  double elapsed;
  {
    std::lock_guard lock(g_delta_mutex);
    if (g_delta_calls.size() > 256) {
      std::erase_if(g_delta_calls, [&](const auto& call) { return now - call.second > std::chrono::seconds(1); });
    }
    auto [it, first] = g_delta_calls.try_emplace(script, now);
    elapsed = first ? 0.0 : std::chrono::duration<double>(now - it->second).count();
    it->second = now;
  }
  if (elapsed <= 0.0 || elapsed > 4.0 / 60.0) {
    elapsed = 1.0 / 60.0;
  }
  uint32_t bits = LoadWord(base, kFrameTime);
  float frame;
  std::memcpy(&frame, &bits, 4);
  const double scale = frame > 0.0f ? f1.f64 / frame : 1.0;
  f1.f64 = double(float(elapsed * scale));
}

// Button presses polled by scripts: the script function ControllerMake (sub_82283AF8) tests the bits of the buttons
// pressed since the previous update (+52 of a controller's input state; sub_82230A08 updates them once a frame, +48 =
// held, +52 = pressed, +56 = released), so a press shows for one frame. A script whose game-frame waits are held (above)
// runs only every second frame or so at 120 Hz and would miss the presses of the frames between (GH Metallica: the
// calibration's strums counted only sometimes at 160). Each script is given the presses of the updates since its
// previous run that it did not see, once, from the last 60th of a second and a frame: they are added to the state's
// pressed bits while its ControllerMake runs, then the bits are put back.
namespace {

constexpr int kKeptUpdates = 8;

struct InputUpdates {
  uint32_t state = 0;  // the input state's guest address
  uint32_t seq = 0;    // updates so far
  uint32_t pressed[kKeptUpdates] = {};
  std::chrono::steady_clock::time_point when[kKeptUpdates] = {};
};

struct ScriptPolls {
  uint32_t run_seq = 0;   // the state's update at the script's current run
  uint32_t prev_seq = 0;  // and at its previous run
  std::chrono::steady_clock::time_point last;
};

struct PatchedState {
  uint32_t state, written, original;
};

std::mutex g_input_mutex;
InputUpdates g_input[4];
std::unordered_map<uint64_t, ScriptPolls> g_script_polls;  // by script << 32 | state
thread_local PatchedState g_patched[4];
thread_local int g_patched_count = 0;

}  // namespace

// [[midasm_hook]] 0x82230BCC (sub_82230A08, after the input state's held, released and pressed bits are stored):
// r30 = the state, r11 = the pressed bits.
void GHAerosmithInputUpdateHook(PPCRegister& r30, PPCRegister& r11) {
  if (!HighRate()) {
    return;
  }
  std::lock_guard lock(g_input_mutex);
  for (InputUpdates& input : g_input) {
    if (input.state == r30.u32 || input.state == 0) {
      input.state = r30.u32;
      const int slot = int(++input.seq % kKeptUpdates);
      input.pressed[slot] = r11.u32;
      input.when[slot] = std::chrono::steady_clock::now();
      return;
    }
  }
}

// [[midasm_hook]] at the entry of sub_82283AF8 (ControllerMake): r4 = the script.
void GHAerosmithButtonPollHook(PPCRegister& r4) {
  g_patched_count = 0;
  uint8_t* base = GuestBase();
  if (!HighRate() || !base) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const auto window = std::chrono::duration<double>(1.0 / 60.0 + FrameSeconds());
  std::lock_guard lock(g_input_mutex);
  if (g_script_polls.size() > 256) {
    std::erase_if(g_script_polls, [&](const auto& entry) { return now - entry.second.last > std::chrono::seconds(1); });
  }
  for (const InputUpdates& input : g_input) {
    if (input.state == 0) {
      continue;
    }
    ScriptPolls& polls = g_script_polls[uint64_t(r4.u32) << 32 | input.state];
    if (polls.last == std::chrono::steady_clock::time_point{} || now - polls.last > window * 4) {
      polls.run_seq = polls.prev_seq = input.seq;  // a script new to polling: only the current update
    } else if (polls.run_seq != input.seq) {
      polls.prev_seq = polls.run_seq;
      polls.run_seq = input.seq;
    }
    polls.last = now;
    uint32_t missed = 0;
    const uint32_t kept = polls.run_seq >= kKeptUpdates ? polls.run_seq - kKeptUpdates + 1 : 1;
    for (uint32_t seq = std::max(polls.prev_seq + 1, kept); seq < polls.run_seq; ++seq) {
      const int slot = int(seq % kKeptUpdates);
      if (now - input.when[slot] <= window) {
        missed |= input.pressed[slot];
      }
    }
    const uint32_t original = LoadWord(base, input.state + 52);
    if (missed & ~original) {
      StoreWord(base, input.state + 52, original | missed);
      g_patched[g_patched_count++] = {input.state, original | missed, original};
    }
  }
}

// [[midasm_hook]] 0x82283B7C, 0x82283BB4 and 0x82283BC0 (sub_82283AF8's three exits): the pressed bits put back.
void GHAerosmithButtonPollEndHook() {
  uint8_t* base = GuestBase();
  for (int i = 0; i < g_patched_count && base; ++i) {
    const PatchedState& patched = g_patched[i];
    if (LoadWord(base, patched.state + 52) == patched.written) {
      StoreWord(base, patched.state + 52, patched.original);
    }
  }
  g_patched_count = 0;
}

// The physics (Havok 4.6): the main loop (sub_82156D68 -> sub_82250750) steps the world by a fixed 1/60 s each frame
// (sub_8269A0C8 and sub_8269A0D0, the constant at 0x82001220), while the bodies that follow the characters' animated
// bones (the roots of the hair, the clothes and the guitar cables, which hang from them by constraints) are moved by
// hkpKeyFrameUtility::applyHardKeyFrame (sub_82697090: velocity = (target - position) / dt) with the frame's own time
// (sub_82175D40; from sub_821A2250, sub_82349B88, sub_823FFD88 and sub_824005E8). At 60 FPS each step brings a body to
// its target. At 160 a step of 1/60 s carries it 2.67 times as far, past the target, and further back the next frame:
// the bodies swung from side to side at about 170 m/s (a temporary probe) and the hair and the cables stretched across
// the stage in spikes (Gustavo, at 120 and 160 FPS; it settles below about 100, where a step carries a body less than
// twice as far). Above 60 Hz the world is stepped by the frame's time when it is shorter than 1/60 s, the time the
// keyframes are given: the bodies reach their targets again and the physics keeps real time (it ran 2.67 times as fast).
// [[midasm_hook]] 0x82250794 (sub_82250750: after "lfs f31,4640(r11)", the step's 1/60 s, before it is passed to
// sub_8269A0C8 and sub_8269A0D0): f31 = the step.
void GHAerosmithPhysicsStepHook(PPCRegister& f31) {
  uint8_t* base = GuestBase();
  if (!base || !HighRate()) {
    return;
  }
  const uint32_t bits = LoadWord(base, kFrameTime);  // sub_82175D40's
  float frame;
  std::memcpy(&frame, &bits, 4);
  if (frame > 0.0f && frame < f31.f64) {
    f31.f64 = frame;
  }
}

// The loading screen (sub_82394BD0, drawn every frame while it is up) animates by the frame, not by time, as GH
// Metallica's:
// - the wheel DisplayLoadingScreen gives it (load_wheel at the boot; sub_82394968: the vertices of its mesh turned about
//   their center by 0.175 radians each frame, 10.5 a second at 60);
// - in its mode 2 a grid waves with two phases that advance by constants each frame.
// Above 60 Hz the steps are scaled by 60 / the rate.
namespace {

double FrameStepScale() { return 60.0 / VblankRate(); }

}  // namespace

// [[midasm_hook]] 0x82394A90 (sub_82394968): after "lfd f31,-11784(r11)", f31 = the wheel's turn in radians each frame.
void GHAerosmithLoadingWheelHook(PPCRegister& f31) {
  if (HighRate()) {
    f31.f64 *= FrameStepScale();
  }
}

// [[midasm_hook]] 0x82394E58 and 0x82394E68 (sub_82394BD0's mode 2): before "fadds f0,f0,f29" and "fadds f0,f0,f13",
// f29 / f13 = the step each wave phase advances by.
void GHAerosmithLoadingWaveHook(PPCRegister& step) {
  if (HighRate()) {
    step.f64 = double(float(step.f64 * FrameStepScale()));
  }
}
