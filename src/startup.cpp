// ghaerosmith - startup (see startup.h).

#include "startup.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/system/flags.h>

#include <algorithm>
#include <cwctype>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "profile_store.h"
#include "project.h"

namespace ghaerosmith {
namespace {

// Whether `folder` is inside the repository the build was made from (the parent of its assets\ folder): a
// development build run from out\build\. A copy anywhere else (a release package, an installation) is not, even
// on the machine that built it.
bool InsideRepository(const std::filesystem::path& folder, const std::filesystem::path& repository_assets) {
  std::error_code ec;
  auto normal = [&ec](const std::filesystem::path& p) {
    std::wstring s = std::filesystem::weakly_canonical(p, ec).generic_wstring();
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return wchar_t(std::towlower(c)); });
    if (!s.empty() && s.back() != L'/') s += L'/';
    return s;
  };
  std::wstring root = normal(repository_assets.parent_path());
  std::wstring inside = normal(folder);
  return root.size() > 1 && inside.compare(0, root.size(), root) == 0;
}

// CRC32 of the default.xex this code was recompiled from (10,760,192 bytes, title ID 41560819, version 0.0.0.1,
// media ID 37A41C2D).
constexpr uint32_t kGameExecutableCrc32 = 0x529115C6;

// The GPU settings KameoRecomp, GRAWRecomp and POTCAWERecomp share, none shown to be needed by this game yet:
// when something draws wrong, try the SDK default of each one from the command line first. Each comment says what
// the setting does and what the other value gives back.
constexpr const char kDefaultConfig[] =
    R"(# GHAerosmithRecomp.exe runtime configuration, loaded at startup. The game recreates this file with these
# defaults if it is missing, and adds the settings an existing file lacks. GH Aerosmith Launcher saves its choices to
# ghaerosmith_options.toml, which is loaded after this file; command line options (--name=value) win over both.
log_level = "info"
# Do not wait at the end of the frame for new D3D12 pipelines (a stutter the first time an effect
# appears): the object appears a few frames later. false = wait.
gpu_async_pipelines_no_frame_wait = true
# Translate the shaders' control flow with if blocks instead of a switch. In POTCAWERecomp the
# switch (the SDK's default) made the pixel shader body re-execute under GPU load and lit objects
# turned into flat saturated colors. true = the SDK default.
dxbc_switch = false
# Clears drawn through a surface with another MSAA mode or pitch than the scene: without these
# settings the SDK copies the render target into that alias and back around each clear. Run the
# vertex shader of unclipped draws on the CPU to know how much EDRAM they use (the Xenia default;
# the SDK default is false)...
execute_unclipped_draw_vs_on_cpu = true
# ...and skip the copy into a render target when the draw replaces every sample of it.
render_target_skip_transfers_on_overwrite = true
# Write constant full-coverage draws (clears) straight into the render targets that hold that
# memory. Off: in KameoRecomp it removed all the geometry from a cinematic (a bug not found yet).
render_target_draw_clears = false
# Keep the GPU copies of memory the CPU uploaded valid across frames. With the SDK default (true)
# every page uploaded by the CPU is re-uploaded the next frame even if it did not change. false is
# the original Xenia behavior (true is a hack for one game).
clear_memory_page_state = false
# Anisotropic filtering for textures with mipmaps, sharper at oblique angles: 5 = 16x, 4 = 8x,
# 3 = 4x (the SDK default), 2 = 2x, 0 = off, -1 = only what the game asks. GH Aerosmith Launcher's
# "Anisotropic filtering" row sets it.
anisotropic_override = 5
# Call the game's audio callback at least 3 ms apart (the console calls it every 5.3 ms), not in the
# bursts of two the host device frees frames in. FMOD's mixer polls how far its two-block ring has
# been read and missed most blocks: from the title screen on, the music played as noise. 0 = the SDK
# default (no spacing).
audio_callback_min_interval_us = 3000
# Audio frames queued for the sound device (5.3 ms each): 4 is the least the SDK allows, 21 ms less
# delay between a strum and its sound than the SDK's 8. With the calls spaced the queue stays full
# (no gap in a whole song, measured); the game's calibration covers what is left. 8 = the SDK default.
audio_maxqframes = 4
# The SDK's audio threads (its audio worker runs the game's sound callback) in Windows' multimedia class "Pro Audio",
# as the sound device's thread: a busy PC holds them up less. false = normal priority (the SDK default).
audio_thread_priority = true
# When the game's sound still comes late (its audio thread held up longer than the 21 ms queue), the silence played
# meanwhile is made up by dropping as many of its next frames: a short skip, and the music stays with the notes (the
# SDK default, false, plays the rest of the song that much late, until a pause).
audio_keep_time = true
# GH Aerosmith Launcher's "Low latency audio" (audio_wasapi_exclusive, off by default) takes the output device in
# exclusive mode, past Windows' audio engine. With it, 10 ms kept in the device's buffer and 2 of the game's frames
# queued instead of 4: on a Focusrite Scarlett 2i2, 20 ms from the game's frame to the driver playing it, where the
# shared output takes 77 ms (measured in GH Metallica). Without it these do nothing.
audio_wasapi_exclusive_buffer_ms = 10
audio_wasapi_exclusive_queue = 2
# V-Sync (GH Aerosmith Launcher's row writes both): NativeXRenderer presents with FIFO, each frame shown once and never
# torn, and the game's vertical blank follows the monitor's where the frame rate is its refresh rate or a whole
# fraction of it, so the game never runs ahead of the display. Measured in GH Metallica at 160 FPS on a 160 Hz display:
# 4 ms from present to display, no frame dropped. "immediate" and false = V-Sync off (tearing, or a G-Sync / FreeSync
# display following the game).
native_x_renderer_present_mode = "vsync"
vblank_follows_display = true
# V-Sync in a window that does not cover its display (NativeXRenderer D182; GH Warriors of Rock's and GH5's setting):
# MAILBOX there, the compositor or a display plane showing one whole picture a refresh. Measured in GH WoR at 160 FPS
# in a 1280x720 window on a display plane: every frame shown, the shown frames' 1 % low 158.3-158.7 FPS (FIFO
# 151.6-158.6) and 2.6-3.1 ms from present to display (FIFO 2.4-6.2, by its phase). Borderless and full screen keep
# FIFO (MAILBOX there: 10.4 ms to the display against 4.0). "vsync" = FIFO in a window too.
native_x_renderer_window_present_mode = "mailbox"
# NativeXRenderer records its DiligentCore calls on a thread of their own while its renderer's thread prepares the next
# commands (D154, D155), on as in GH Warriors of Rock and GH5 (GH WoR's Quest song at 3440x1440: at 240 FPS every
# stretch at 230 or more, 225.7 without it). With V-Sync, D172 drops one frame when the display's queue stays a frame
# too deep. false = one thread.
native_x_renderer_record_thread = true
# Where the vertical blank runs on the SDK's timer (a G-Sync / FreeSync display, V-Sync off): its ticks within a few
# microseconds of their time (700 us of spinning each; without, up to half a millisecond late), as the display shows
# each frame when it comes.
vblank_timer_spin_us = 700
# A controller (and the keyboard's emulated one) plays as a controller, with Guitar Hero III's own layout (the
# game's code, found 2026-11-13; docs/fixes-from-gh-metallica.md): the frets on LT (green), LB (red), RB (yellow), RT
# (blue) and A (orange), and no strum: a fret pressed plays its note (the game takes a change of the frets held as the
# strum); Star Power on the D-pad (any direction) or Back; the whammy bar on either stick, pushed from the centre.
# 7 = reported as a Guitar Hero guitar instead (the frets on A, B, Y, X and LB, strumming on the D-pad, as GH Metallica's
# recompilation plays a controller); 0 = the SDK default, a gamepad.
gamepad_subtype = 0
# Report a guitar as Guitar Hero's (alternate guitar). SDL reports every guitar as a plain guitar, Rock
# Band's kind, which this game calls an unsupported peripheral and does not let play. 0 = the SDK default.
guitar_subtype = 7
# An Xbox 360 guitar reads its whammy bar half pressed until the bar first moves: at rest until then.
guitar_whammy_rest = true
# A PS3 Guitar Hero guitar (a CRKD guitar on its P3 setting too): yellow and blue, swapped on that protocol, put right,
# and its whammy bar, which rests at the middle of its axis, at rest there.
guitar_ps3_protocol = true
# Start pressed on a second controller: the game asks that controller to sign in, and only the first
# has a profile. The sign-in UI closes as when the player cancels it, and the game shows its "not
# signed in" message (sign in / continue without saving); the SDK default reported a sign-in change
# and the game waited for that controller's profile forever on an empty screen.
signin_ui_cancel = true
# The other controllers are the profile's guests, as on the console (Name (1)): signed in, so the game
# does not ask them to sign in, with no saves or settings of their own. The controller that presses a
# button at the title screen is the profile's (ghaerosmith_profile_follows_press, src/players.cpp).
signin_guests = true
# The keyboard plays as a guitar in the menus and on Guitar Hero III's controller layout in a song (SDK change 126,
# keyboard_guitar_layout = "gh3" for a keyboard the game is told is a gamepad; GH5's keyboard): in the menus the fret
# keys are a guitar's (green selects, red goes back), in a song (the tap gate open, src/keyboard.cpp) the SDK gives the
# game the green, red, yellow, blue and orange keys (A, B, Y, X and LB below) on LT, LB, RB, RT and A, where this game's
# controller code reads its frets: a fret key pressed plays, no strum (the arrows do nothing there), Star Power on
# Back's keys, the whammy bar on the right stick's key. Empty: the keys below are a plain controller's everywhere (D on
# A: the orange fret in a song).
keyboard_guitar_layout = "gh3"
# The keyboard as a guitar (with mnk_mode; GH Aerosmith Launcher's Keyboard page changes them), as in GH5 and GH
# Metallica: the frets on D, F, J, K and L (green, red, yellow, blue, orange: A, B, Y, X, LB); in the menus D selects,
# F goes back and the arrows move; Star Power on Space or Backspace (Back); Start on Enter or X; the whammy bar on the
# right arrow (the right stick). The triggers, RB and the sticks' other keys are off: W A S D, F and K would move the
# sticks with the frets.
keybind_left_trigger = ""
keybind_left_shoulder = "L"
keybind_right_shoulder = ""
keybind_right_trigger = ""
keybind_a = "D"
keybind_b = "F"
keybind_y = "J"
keybind_x = "K"
keybind_dpad_up = "Up"
keybind_dpad_down = "Down"
keybind_dpad_left = "Shift+Left"
keybind_dpad_right = "Shift+Right"
keybind_back = "Space,Backspace"
keybind_start = "Return,X"
keybind_rstick_right = "Right"
keybind_rstick_up = ""
keybind_rstick_down = ""
keybind_rstick_left = ""
keybind_rstick_press = ""
keybind_lstick_up = ""
keybind_lstick_down = ""
keybind_lstick_left = ""
keybind_lstick_right = ""
keybind_lstick_press = ""
# The SDK's developer keys (F3 the debug overlay, F4 its settings window, ` the console) do nothing, so that
# the keyboard can play with them (a player's request in GH Metallica, 2026-11-02); GH Aerosmith Launcher's
# Settings > Game > Debug keys turns them on. A key on one of the keyboard's controls plays only that control either
# way, F7 (the achievements) included.
debug_hotkeys = false
# The videos at the start (the logos, the game's intro) skipped: the game starts at its title screen once it has loaded
# (src/intro.cpp). false = as on the console.
ghaerosmith_skip_intro = false
# The game's own debug menu (Neversoft's, src/debug_menu.cpp): true adds a Debug item to the main menu and the pause
# menu. The development shortcuts the same switch had (unlocking or completing songs from the setlist, the store's
# money) stay off. Some items need Neversoft's development tools and do nothing. false = as on the console.
ghaerosmith_debug_menu = false
# The note highway (src/mods.cpp): its size with one player, its length in time and its perspective, in percent (100 = as
# on the console); with a perspective other than 100, keep_speed makes the notes reach the strike line at the console's
# speed.
ghaerosmith_highway_scale = 100
ghaerosmith_highway_length = 100
ghaerosmith_highway_perspective = 100
ghaerosmith_highway_keep_speed = false
# The note highway's look (src/mods.cpp): "original", or "dark" (the community's black highway).
ghaerosmith_highway_style = "original"
# The note highway's opacity in percent: below 100 the venue shows through it (src/mods.cpp). 100 = as on the console.
ghaerosmith_highway_opacity = 100
# The whammy bar's sound (src/mods.cpp): false keeps a held sustain's pitch (the bar still wobbles the note and fills
# Star Power). true = as on the console.
ghaerosmith_whammy_sound = true
# Your instrument's track on a miss or a strum without a note (src/mods.cpp): lowered to miss_volume percent (0 =
# muted as on the console), or with miss_lower = false left at full volume.
ghaerosmith_miss_lower = true
ghaerosmith_miss_volume = 0
# The mouse cursor hidden over the game after cursor_auto_hide_ms without moving it (shown again when it moves, while
# another window has the focus, and over the SDK's overlays). false = always shown.
cursor_auto_hide = true
cursor_auto_hide_ms = 2000
)";

uint32_t Crc32(const std::vector<char>& data) {
  static const std::array<uint32_t, 256> table = [] {
    std::array<uint32_t, 256> t{};
    for (uint32_t i = 0; i < 256; ++i) {
      uint32_t c = i;
      for (int k = 0; k < 8; ++k) {
        c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      }
      t[i] = c;
    }
    return t;
  }();
  uint32_t crc = 0xFFFFFFFFu;
  for (char byte : data) {
    crc = table[(crc ^ uint8_t(byte)) & 0xFF] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFFu;
}

#ifdef _WIN32
// MB_* flags; returns the button pressed.
int ShowMessage(const std::wstring& text, UINT flags) {
  return MessageBoxW(nullptr, text.c_str(), L"" GAME_TITLE, flags);
}

std::wstring Hex32(uint32_t value) {
  wchar_t buffer[16];
  swprintf(buffer, 16, L"%08X", value);
  return buffer;
}
#endif

}  // namespace

std::filesystem::path DefaultGameRoot() {
  std::filesystem::path next_to_exe = rex::filesystem::GetExecutableFolder() / "assets";
  if (std::filesystem::exists(next_to_exe / GAME_XEX)) {
    return next_to_exe;
  }
  // The repository's assets\, for a development build run from the repository only: a package or an installation
  // uses its own (the launcher extracts the game there), even on the machine that built it.
  std::filesystem::path repository = GHAEROSMITH_DEFAULT_GAME_ROOT;
  if (InsideRepository(rex::filesystem::GetExecutableFolder(), repository) &&
      std::filesystem::exists(repository / GAME_XEX)) {
    return repository;
  }
  // Neither has the game: report the folder next to the executable (the release layout), not a path
  // of the machine the build was made on.
  return next_to_exe;
}

void ConfigureUserData(std::filesystem::path& user_data_root, std::filesystem::path& cache_root) {
  if (!rex::cvar::Query<std::string>("user_data_root").empty()) {
    return;  // chosen on the command line
  }
  std::filesystem::path portable = rex::filesystem::GetExecutableFolder() / "userdata";
  std::error_code ec;
  std::filesystem::create_directories(portable, ec);
  // Writable? Logging is not set up yet: the folder in use is logged later
  // (GhaerosmithApp::SetupEnvironment).
  std::filesystem::path probe = portable / ".write_test";
  {
    std::ofstream f(probe, std::ios::binary);
    if (!f) {
      return;
    }
  }
  std::filesystem::remove(probe, ec);
  user_data_root = portable;
  if (rex::cvar::Query<std::string>("cache_root").empty()) {
    cache_root = portable / "cache";
  }
}

void ConfigureProfile(const std::filesystem::path& user_data_root) {
  if (!REXCVAR_GET(profile_root).empty()) {
    return;  // chosen on the command line
  }
  if (std::optional<launcher::profiles::GameProfile> profile = launcher::profiles::ForGame(user_data_root)) {
    std::u8string folder = profile->folder.u8string();
    REXCVAR_SET(profile_root, std::string(folder.begin(), folder.end()));
    if (REXCVAR_GET(profile_name).empty()) {
      REXCVAR_SET(profile_name, profile->name);
    }
  }
}

struct ChangedDefault {
  const char* key;
  const char* old_value;
};
constexpr ChangedDefault kChangedDefaults[] = {
    // 2026-11-13: controllers on the game's own controller layout (gamepad_subtype 7 -> 0).
    {"gamepad_subtype", "7"},
    // 2026-11-24: the keyboard back on a guitar's keys (keyboard_guitar_layout = "gh3"), from the controller layout of
    // 2026-11-13, whose D (LT) the menus did not take. The keys before 2026-11-13 were a guitar's already.
    {"keybind_left_trigger", "\"D\""},
    {"keybind_left_shoulder", "\"F\""},
    {"keybind_right_shoulder", "\"J\""},
    {"keybind_right_trigger", "\"K\""},
    {"keybind_a", "\"L,Return\""},
    {"keybind_b", "\"Backspace\""},
    {"keybind_y", "\"\""},
    {"keybind_x", "\"\""},
    {"keybind_dpad_left", "\"Left\""},
    {"keybind_dpad_right", "\"Right\""},
    {"keybind_back", "\"Space\""},
    {"keybind_start", "\"X,P\""},
    {"keybind_rstick_right", "\"RShift\""},
};

// The lines of `text` split at '\n' (a last piece without one included; joined back with '\n', the text again).
std::vector<std::string> SplitLines(const std::string& text) {
  std::vector<std::string> lines;
  size_t start = 0;
  for (;;) {
    size_t end = text.find('\n', start);
    if (end == std::string::npos) {
      lines.push_back(text.substr(start));
      return lines;
    }
    lines.push_back(text.substr(start, end - start));
    start = end + 1;
  }
}

// The index of `key`'s line (`key = value`, spaces around) in `lines`, its value trimmed in `value`; -1 when absent.
int FindSettingLine(const std::vector<std::string>& lines, const std::string& key, std::string* value) {
  for (size_t i = 0; i < lines.size(); ++i) {
    const std::string& line = lines[i];
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos || line.compare(first, key.size(), key) != 0) {
      continue;
    }
    size_t eq = line.find_first_not_of(" \t", first + key.size());
    if (eq == std::string::npos || line[eq] != '=') {
      continue;
    }
    std::string v = line.substr(eq + 1);
    v.erase(0, v.find_first_not_of(" \t"));
    v.erase(v.find_last_not_of(" \t\r") + 1);
    if (value) {
      *value = v;
    }
    return int(i);
  }
  return -1;
}

// The first line of the comments right above line `index`.
size_t CommentStart(const std::vector<std::string>& lines, size_t index) {
  while (index > 0) {
    const std::string& above = lines[index - 1];
    size_t first = above.find_first_not_of(" \t");
    if (first == std::string::npos || above[first] != '#') {
      break;
    }
    --index;
  }
  return index;
}

// Replaces in `existing` the settings of kChangedDefaults that still have their old value; true when one was.
bool ApplyChangedDefaults(std::string& existing) {
  std::vector<std::string> lines = SplitLines(existing);
  const std::vector<std::string> defaults = SplitLines(kDefaultConfig);
  bool changed = false;
  for (const ChangedDefault& setting : kChangedDefaults) {
    std::string value;
    int line = FindSettingLine(lines, setting.key, &value);
    int default_line = FindSettingLine(defaults, setting.key, nullptr);
    if (line < 0 || default_line < 0 || value != setting.old_value) {
      continue;
    }
    size_t from = CommentStart(lines, size_t(line));
    size_t default_from = CommentStart(defaults, size_t(default_line));
    lines.erase(lines.begin() + from, lines.begin() + line + 1);
    lines.insert(lines.begin() + from, defaults.begin() + default_from, defaults.begin() + default_line + 1);
    changed = true;
  }
  if (changed) {
    existing.clear();
    for (size_t i = 0; i < lines.size(); ++i) {
      existing += (i ? "\n" : "") + lines[i];
    }
  }
  return changed;
}

// Creates ghaerosmith.toml with the defaults, or appends to an existing one the settings (with their
// comments) it does not mention, so that a setting added in a newer build takes effect without
// deleting the user's file. A setting the file has, with any value, is left alone, but for a changed default the
// file still has the old value of (kChangedDefaults).
void EnsureDefaultConfig(const std::filesystem::path& config_path) {
  // Logging is not initialized yet (the config sets it up): nothing to log here.
  std::string existing;
  if (std::filesystem::exists(config_path)) {
    std::ifstream in(config_path, std::ios::binary);
    existing.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  } else {
    std::ofstream f(config_path, std::ios::binary);
    f << kDefaultConfig;
    return;
  }
  const bool migrated = ApplyChangedDefaults(existing);
  auto has_key = [&existing](const std::string& key) {
    size_t pos = 0;
    while ((pos = existing.find(key, pos)) != std::string::npos) {
      size_t line_start = existing.rfind('\n', pos);
      line_start = line_start == std::string::npos ? 0 : line_start + 1;
      size_t after = pos + key.size();
      bool only_spaces_before = existing.find_first_not_of(" \t", line_start) == pos;
      size_t next = existing.find_first_not_of(" \t", after);
      if (only_spaces_before && next != std::string::npos && existing[next] == '=') {
        return true;
      }
      pos = after;
    }
    return false;
  };
  std::string missing;
  std::string comments;
  std::istringstream defaults(kDefaultConfig);
  std::string line;
  while (std::getline(defaults, line)) {
    if (line.empty()) {
      comments.clear();
      continue;
    }
    if (line[0] == '#') {
      comments += line + '\n';
      continue;
    }
    size_t eq = line.find('=');
    if (eq == std::string::npos) {
      comments.clear();
      continue;
    }
    std::string key = line.substr(0, eq);
    key.erase(key.find_last_not_of(" \t") + 1);
    if (!has_key(key)) {
      missing += comments + line + '\n';
    }
    comments.clear();
  }
  if (missing.empty() && !migrated) {
    return;
  }
  std::ofstream f(config_path, std::ios::binary | (migrated ? std::ios::trunc : std::ios::app));
  if (migrated) {
    f << existing;
  }
  if (!missing.empty()) {
    if (!existing.empty() && existing.back() != '\n') {
      f << '\n';
    }
    f << missing;
  }
}

bool CheckGameFiles(const std::filesystem::path& game_root) {
  std::filesystem::path exe = game_root / GAME_XEX;
  std::ifstream f(exe, std::ios::binary);
  if (!f) {
    REXLOG_ERROR("[startup] Game files not found: {} does not exist", exe.string());
#ifdef _WIN32
    ShowMessage(L"Game files not found.\n\n"
                L"Copy the files of your " GAME_TITLE " (Xbox 360) disc into:\n" +
                    game_root.wstring() + L"\n\nso that this file exists:\n" + exe.wstring(),
                MB_OK | MB_ICONERROR);
#endif
    return false;
  }
  std::vector<char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  uint32_t crc = Crc32(data);
  if (crc == kGameExecutableCrc32) {
    REXLOG_INFO("[startup] {}: CRC32 {:08X} (expected version)", exe.string(), crc);
    return true;
  }
  REXLOG_WARN("[startup] {}: CRC32 {:08X}, expected {:08X}: a different version of the game",
              exe.string(), crc, kGameExecutableCrc32);
#ifdef _WIN32
  int answer = ShowMessage(L"" GAME_XEX " is not the version this build was recompiled from "
                           L"(CRC32 " + Hex32(crc) + L", expected " + Hex32(kGameExecutableCrc32) +
                               L").\nThe game will most likely crash.\n\nStart anyway?",
                           MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
  return answer == IDYES;
#else
  return true;
#endif
}

}  // namespace ghaerosmith
