// ghaerosmith - the game's threads wait without spinning.
//
// Loops of the game wait by checking a condition again and again, which on the console costs a hardware thread each
// and nothing else. Here two of them kept a core at 100 % each (2.4 cores in the main menu, measured with
// recomp-tools' threadcpu.ps1 and stacksample.ps1), as in GH Metallica (GHMetalRecomp's src/thread_waits.cpp):
// - The main thread (xstart -> sub_820D2408 -> sub_82156D68 -> sub_822C3808 -> sub_822C36A0), at the end of each
//   frame: until the vertical blank counter sub_82175D30 returns (0x82AE38D8) reaches the frame's last vertical blank
//   (the frame interval, 0 to 2, at 0x82BF4AF0), then while (frames[24] == frames[20]) {} until the render thread has
//   finished a frame.
// - The render thread (sub_824C0110 -> sub_82386860 -> sub_823860F0 -> sub_822CE808, the frame's presentation),
//   waiting twice per frame: until the counter sub_82175CF0 returns (0x82AE38E0) passes the value it saved the
//   previous frame, and until the vertical blank counter reaches the frame's last vertical blank; the counters are
//   advanced by the game's vertical blank interrupt.
// The hooks in ghaerosmith_functions.toml leave each loop as it is and slow its checks down: a wait first spins
// briefly (short waits are unchanged), and then the thread sleeps in short slices between checks. With
// ghaerosmith_wait_smart (on) the vertical blank waits sleep in one go until shortly before the vertical blank they
// wait for, which comes at the video mode's refresh rate (1/60 s by default): slices woke each thread thousands of
// times a second in GH Metallica, and each wake costs the kernel a timer and a context switch. Then they block on an
// event the game's two callbacks set right after they advance the counters (sub_82175BF0, the swap's: 0x82AE38E0;
// sub_82175B98, the vertical blank's: 0x82AE38D8): a waiting thread wakes some tens of microseconds after the counter,
// not up to a slice after, and a variable refresh rate display (G-Sync, FreeSync) shows each frame as regularly as the
// vertical blanks come (GH Metallica's of 2026-11-04: its frame time made a saw tooth on Gustavo's G-Sync displays).
//
// The D3D library's waits for the GPU (ten loops: the fences, the present, ...; each calls sub_824DCC98 for every
// check, a delay of 32 db16cyc and a hang check) spun too: at 120 FPS GH Metallica's render thread spent half its time
// in the present's. They spin and sleep in slices like the loops above.

#include <rex/cvar.h>
#include <rex/ppc/context.h>
#include <rex/thread.h>

#include <immintrin.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>

REXCVAR_DEFINE_INT32(ghaerosmith_wait_sleep_us, 200, "GHAerosmith",
                     "Game threads waiting for work: sleep this many microseconds per check once the wait "
                     "is longer than ghaerosmith_wait_spin_us (0 = busy wait, as on the console)");
REXCVAR_DEFINE_INT32(ghaerosmith_wait_spin_us, 100, "GHAerosmith",
                     "Game threads waiting for work: busy-wait this long before sleeping");
REXCVAR_DEFINE_BOOL(ghaerosmith_wait_smart, true, "GHAerosmith",
                    "The vertical blank waits sleep until shortly before the vertical blank (false = every wait "
                    "sleeps in slices of ghaerosmith_wait_sleep_us)");

namespace {

using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

// A check that comes more than this long after the previous one on the same thread starts a new wait: the loop
// did other work in between (or the thread went on with something else and came back).
constexpr auto kNewWaitGap = std::chrono::milliseconds(1);

struct WaitState {
  Clock::time_point start;
  Clock::time_point last;
};

bool Smart() { return REXCVAR_GET(ghaerosmith_wait_smart) && REXCVAR_GET(ghaerosmith_wait_sleep_us) > 0; }

// One check of a wait whose condition is still false.
void SpinOrSleep(WaitState& state) {
  const int32_t sleep_us = REXCVAR_GET(ghaerosmith_wait_sleep_us);
  if (sleep_us <= 0) {
    return;
  }
  const Clock::time_point now = Clock::now();
  if (now - state.last > kNewWaitGap) {
    state.start = now;
  }
  state.last = now;
  const int32_t spin_us = std::max(0, REXCVAR_GET(ghaerosmith_wait_spin_us));
  if (now - state.start < std::chrono::microseconds(spin_us)) {
    _mm_pause();
    return;
  }
  rex::thread::PreciseSleep(std::chrono::microseconds(sleep_us));
  state.last = Clock::now();
}

thread_local WaitState t_render_wait;
thread_local WaitState t_gpu_wait;

// A vertical blank counter's last advance seen while a thread was checking it closely, the anchor of when the next
// ones come: the SDK's vblank thread advances it once a period (VblankPeriod), within a millisecond (it checks the
// time every millisecond).
struct Vblank {
  uint64_t value = 0;
  Clock::time_point checked;   // the previous check
  uint64_t anchor_value = 0;   // 0: no anchor yet
  Clock::time_point anchor_time;
  WaitState wait;
};
// The main thread's wait and the render thread's two, each counter on its own thread.
thread_local Vblank t_vblank[2];

// Set by the game's callbacks right after they advance the counters, one event per wait (auto-reset: one waiter each):
// 0 the render thread's wait for the swap counter (0x82AE38E0, sub_82175BF0), 1 its wait for the vertical blank counter
// (0x82AE38D8, sub_82175B98), 2 the main thread's for the same counter (its frame end). A waiter that finds its event set
// from an earlier advance checks its counter again and blocks again.
HANDLE g_counter_event[3] = {CreateEventW(nullptr, FALSE, FALSE, nullptr), CreateEventW(nullptr, FALSE, FALSE, nullptr),
                             CreateEventW(nullptr, FALSE, FALSE, nullptr)};
std::atomic<bool> g_counter_signaled[3];

void SignalCounter(int index) {
  g_counter_signaled[index].store(true, std::memory_order_relaxed);
  SetEvent(g_counter_event[index]);
}

// How long before the expected advance the long sleep ends, for the vblank thread's millisecond and the sleep's
// own lateness; checked in slices from there.
constexpr auto kVblankMargin = 2ms;

// The vertical blank's period as the SDK's vblank thread keeps it: 1 / video_mode_refresh_rate with vsync (60 Hz by
// default), or none without vsync (a vblank every millisecond: nothing to predict). Read once a second.
std::chrono::nanoseconds VblankPeriod() {
  thread_local std::chrono::nanoseconds period{0};
  thread_local Clock::time_point read;
  const Clock::time_point now = Clock::now();
  if (read == Clock::time_point{} || now - read > 1s) {
    read = now;
    const double hz = std::clamp(rex::cvar::Query<double>("video_mode_refresh_rate"), 24.0, 1000.0);
    period = rex::cvar::Query<bool>("vsync") ? std::chrono::nanoseconds(int64_t(1e9 / hz)) : std::chrono::nanoseconds(0);
  }
  return period;
}

void VblankWait(Vblank& state, int index, uint64_t counter, uint64_t target) {
  const Clock::time_point now = Clock::now();
  if (counter != state.value) {
    // An advance seen within a slice of the previous check is a good anchor; one found after other work is not
    // (it may have come long before).
    if (now - state.checked < 500us) {
      state.anchor_value = counter;
      state.anchor_time = now;
    }
    state.value = counter;
  }
  state.checked = now;
  if (counter >= target) {
    return;
  }
  // Once the callback's hook has signaled the counter: until its next advance (a few milliseconds at most; the timeout
  // only in case an advance came some other way).
  if (Smart() && g_counter_event[index] && g_counter_signaled[index].load(std::memory_order_relaxed)) {
    WaitForSingleObject(g_counter_event[index], 4);
    state.checked = state.wait.last = Clock::now();
    return;
  }
  // Predicted only from a recent anchor a few vblanks before the target: one found long ago, or with the counter far
  // ahead of it, says nothing about the next advance. And never more than a period in one sleep.
  const std::chrono::nanoseconds period = Smart() ? VblankPeriod() : std::chrono::nanoseconds(0);
  if (period.count() > 0 && state.anchor_value && target > state.anchor_value &&
      target - state.anchor_value <= 4 && now - state.anchor_time < 8 * period) {
    const Clock::time_point due = state.anchor_time + int64_t(target - state.anchor_value) * period;
    // Above 250 Hz the margin and the least sleep shrink with the period, or no sleep would ever be predicted.
    const auto margin = std::min<std::chrono::nanoseconds>(kVblankMargin, period / 2);
    const auto sleep = std::min<std::chrono::nanoseconds>(due - now - margin, period);
    if (sleep > std::min<std::chrono::nanoseconds>(1ms, period / 4)) {
      rex::thread::PreciseSleep(std::chrono::duration_cast<std::chrono::microseconds>(sleep));
      state.checked = state.wait.last = Clock::now();
      return;
    }
  }
  SpinOrSleep(state.wait);
  state.checked = Clock::now();
}

}  // namespace

// [[midasm_hook]] 0x822C3738 (sub_822C36A0, the main thread's frame end): before "cmpld cr6,r3,r11", r3 = the
// vertical blank counter 0x82AE38D8 just read and r11 the frame's last vertical blank (go on once r3 reaches it).
void GHAerosmithFrameVblankWaitHook(PPCRegister& r3, PPCRegister& r11) { VblankWait(t_vblank[0], 2, r3.u64, r11.u64); }

// [[midasm_hook]] 0x822C3758 (sub_822C36A0, the main thread waiting for the render thread): before
// "cmplw cr6,r11,r10", r11 and r10 = the frame counters (go on once they differ).
void GHAerosmithRenderWaitHook(PPCRegister& r11, PPCRegister& r10) {
  if (r11.u32 == r10.u32) {
    SpinOrSleep(t_render_wait);
  }
}

// [[midasm_hook]] 0x822CE908 (sub_822CE808, the render thread presenting a frame): before "cmpld cr6,r3,r11",
// r3 = the counter 0x82AE38E0 just read and r11 the value it waits for (go on once r3 reaches it).
void GHAerosmithVblankWaitHook(PPCRegister& r3, PPCRegister& r11) { VblankWait(t_vblank[0], 0, r3.u64, r11.u64); }

// [[midasm_hook]] 0x822CEA30 (sub_822CE808): before "cmpld cr6,r3,r11", r3 = the vertical blank counter 0x82AE38D8
// and r11 the frame's last vertical blank.
void GHAerosmithVblankWait2Hook(PPCRegister& r3, PPCRegister& r11) { VblankWait(t_vblank[1], 1, r3.u64, r11.u64); }

// [[midasm_hook]] 0x82175C10 (sub_82175BF0, the swap callback): after "std r10,224(r11)", the swap counter 0x82AE38E0's
// advance.
void GHAerosmithSwapSignalHook() { SignalCounter(0); }

// [[midasm_hook]] 0x82175BC8 (sub_82175B98, the vertical blank callback): after "std r10,0(r7)", the vertical blank
// counter 0x82AE38D8's advance (the render thread's wait and the main thread's).
void GHAerosmithVblankSignalHook() {
  SignalCounter(1);
  SignalCounter(2);
}

// [[midasm_hook]] at the entry of sub_824DCC98: a check of a D3D wait for the GPU whose condition is still false.
void GHAerosmithGpuWaitHook() { SpinOrSleep(t_gpu_wait); }
