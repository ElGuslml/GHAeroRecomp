// This game's profile: everything the launcher knows about the game it belongs to. The launcher's code is the same for
// every game (the RecompLauncher submodule, recomp_launcher\, with game_types.h for the types below); game.h, game.cpp
// (the game's own texts) and palette.h (its colors) are each game's, with the art (launcher\art), the icon
// (res\icon.ico), the resource script (res\launcher.rc) and the version (res\version.h). The names and the options the
// game executable shares are src/project.h's.

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "game_types.h"
#include "project.h"
#include "version.h"

namespace launcher::game {

// --- Names ---

// The game, as the texts name it ("the .iso file of %s").
inline constexpr const char* kName = GAME_TITLE;
// The recompiled executable and the launcher, side by side; the launcher's name is its window's title.
inline constexpr const char* kRecompExecutable = GAME_EXE;
inline constexpr const char* kLauncherExecutable = GAME_LAUNCHER ".exe";
inline constexpr const char* kLauncherName = GAME_LAUNCHER;
// The game's settings next to it: its base file (src/startup.cpp writes the defaults) and what the launcher saves.
inline constexpr const char* kConfigFile = GAME_CONFIG;
inline constexpr const char* kOptionsFile = GAME_OPTIONS;
// The game's user data when its folder cannot be written: Documents\<this> (the SDK's default, the app's name).
inline constexpr const char* kDocumentsFolder = "ghaerosmith";
// res\version.h.
inline constexpr const char* kVersion = RECOMP_VERSION_STR;

// --- The look ---

// The title the Play page shows over the art: a small line above, the name below.
inline constexpr const char* kArtKicker = "GUITAR HERO";
inline constexpr const char* kArtTitle = "Aerosmith";
// The mark in the header's logo while the game's icon is unknown: an icon's SVG paths in a 24-unit box, stroked as the
// launcher's icons are (ui.cpp).
inline constexpr const char* kBrandIcon =
    "<circle cx='12' cy='12' r='9'/><circle cx='12' cy='12' r='2.5'/><path d='M12 3a9 9 0 0 1 9 9'/>";
// The header's name beside the logo, on two lines (in capitals); an empty title: the art's (kArtTitle) alone.
inline constexpr const char* kBrandTitle = "Guitar Hero";
inline constexpr const char* kBrandSubtitle = "Aerosmith";
// Where the art under the pages (launcher\art\background.svg, 1280 x 760: the template's, until the game has its own)
// is cropped in a window of another shape: the point kept in view, as fractions of its width and height.
inline constexpr float kArtFocusX = 0.75f;
inline constexpr float kArtFocusY = 0.6f;
// The logo's mark on a disc instead of a rounded square.
inline constexpr bool kBrandRound = false;
// The titles in capitals (the game's own texts that the launcher shows as titles).
inline constexpr bool kUppercaseTitles = false;

// --- The disc ---

// The disc's executable, at its root and in assets\ once extracted; extracted last, so that its presence marks a
// complete installation.
inline constexpr const char* kDiscExecutable = GAME_XEX;
// The disc this build was recompiled from (src/startup.cpp checks the same CRC at the game's start).
inline constexpr uint32_t kTitleId = 0x41560819;
inline constexpr uint32_t kExecutableCrc32 = 0x529115C6;
inline constexpr const char* kRegion = "USA";
// The game's files on that disc (806 files): the space the first run asks for before an ISO is chosen.
inline constexpr uint64_t kInstalledBytes = 4666046318;
// The game's frame: the Xenos path's scales and NativeXRenderer's internal resolutions are multiples of it.
inline constexpr int kFrameWidth = GAME_FRAME_WIDTH;
inline constexpr int kFrameHeight = GAME_FRAME_HEIGHT;

// --- Updates ---

// This project's releases (updater.cpp): GitHub's API for them, newest first, and their page. Empty: the launcher
// does not look for updates (a private repository), and its Updates page only shows the version. Once the repository
// is public: "https://api.github.com/repos/<user>/GHAeroRecomp/releases?per_page=20" and
// "https://github.com/<user>/GHAeroRecomp/releases".
inline constexpr const char* kReleasesApi = "";
inline constexpr const char* kReleasesPage = "";

// --- The game's own options (a cvar each; empty: the game has no such option, and the launcher no row) ---

// NativeXRenderer draws this game (src/native_x_renderer_ghaerosmith.cpp): the Graphics page's renderer cards, its
// internal resolution, antialiasing and image format. False: the game is drawn by Xenos only, and the launcher says
// nothing of NativeXRenderer.
inline constexpr bool kHasNativeXRenderer = GAME_HAS_NATIVE_X_RENDERER;
// The renderer chosen until the player picks one (an options file without native_x_renderer): NativeXRenderer once a
// full play test has gone through it (as in POTCAWE, its D56), the Xenos path until then.
inline constexpr bool kNativeXRendererDefault = false;
// The upscaling row's AMD FSR 3.1 and NVIDIA DLSS (NativeXRenderer's D47-D49, D116: the main 3D view rebuilt from
// several frames; RecompLauncher 3.6, 3.9): only for a game whose hooks mark its views (native_x_renderer::BeginView
// and EndViewGeometry, as POTCAWE's). Without them DLSS never runs; false: FSR 1 and
// NIS only.
inline constexpr bool kHasDlaa = false;

// The Graphics page's shadow quality row: NativeXRenderer draws the game's shadow maps at a scale of their own
// (native_x_renderer_shadow_scale, its D92): at least twice or four times the game's resolution. Only where it
// finds them: the depth passes with no color target that are not of the game's output size.
// Turned on once the log shows NativeXRenderer finding the game's shadow maps ("shadow map: ...") with
// native_x_renderer_shadow_scale = 4, and their image is right.
inline constexpr bool kShadowQuality = kHasNativeXRenderer && false;

// The Graphics page's frame rate row: kFrameRateCvar's choices (game_types.h's FrameRate) and the value without the
// key. Here src/framerate.cpp's ghaerosmith_frame_rate, the frame rate itself: 60 as on the console, above it the emulated
// vblank's rate (src/options.cpp), which the game follows at its own speed.
inline constexpr const char* kFrameRateCvar = GAME_FRAME_RATE_CVAR;
inline constexpr bool kFrameRateIsBool = false;
// Above 240 (RecompLauncher 3.31, with V-Sync off; as GH Warriors of Rock and GH5): 300, 360, 480 and Unlimited (1000
// Hz, the SDK's ceiling), experimental: the presses kept for held scripts (src/framerate.cpp) reach a 60th of a second
// at 1000. Between 60 and 120 (GH Warriors of Rock's): 72, 75, 90 and 100, for a PC that cannot hold 120; with V-Sync
// they are even on a monitor whose rate they fit (72 on 144 Hz, 75 on 150, 100 on 200), with V-Sync off or a variable
// refresh rate.
inline constexpr std::array<FrameRate, 14> kFrameRates{{{60, 60},
                                                        {72, 72},
                                                        {75, 75},
                                                        {90, 90},
                                                        {100, 100},
                                                        {120, 120},
                                                        {144, 144},
                                                        {160, 160},
                                                        {165, 165},
                                                        {240, 240},
                                                        {300, 300},
                                                        {360, 360},
                                                        {480, 480},
                                                        {1000, 0}}};
inline constexpr int kFrameRateDefault = 60;

// The Xenos path's MSAA turned off (src/msaa.cpp): the Graphics page's antialiasing row with Xenos, the game's own
// antialiasing (its label, "2× MSAA") or none.
inline constexpr const char* kNoMsaaCvar = "";
inline constexpr const char* kMsaaLabel = "";

// The game's cameras widened for a screen wider than 16:9 (src/widescreen.cpp), set with the Video page's image
// format; the Xenos path then stretches its image over the screen (present_letterbox = false). Empty: the format
// applies to NativeXRenderer only.
inline constexpr const char* kUltrawideCvar = GAME_ULTRAWIDE_CVAR;

// The languages the game has (game_types.h's Language): the Game page's language row, in this order. Empty: the game
// asks in its own menu, and the launcher has no row.
// The disc's script and text packs (DATA/COMPRESSED/PAK/qb_*.pab.xen): English and French, German, Italian, Spanish
// (Spain's); the game picks one from XGetLanguage at start, as GH Metallica.
inline constexpr std::array<Language, 5> kLanguages{{
    {3, "Deutsch", nullptr, nullptr},
    {1, "English", nullptr, nullptr},
    {5, "Español", nullptr, nullptr},
    {4, "Français", nullptr, nullptr},
    {6, "Italiano", nullptr, nullptr},
}};

// Switches of the game's own (src/), each a boolean cvar with a row in the Settings tab (game_types.h's Toggle); their
// titles, hints and help are game.cpp's (GameTexts::toggles and toggle_help, in this order). In Settings > Game >
// Starting the game (launcher 3.28's kStart), off as on the console: the boot's videos skipped (src/intro.cpp).
inline constexpr std::array<Toggle, 1> kToggles{{
    {"ghaerosmith_skip_intro", false, ToggleGroup::kStart},
}};

// Rows of the game's own with several values (launcher 3.29): none here.
inline constexpr std::array<Choice, 0> kChoices{};

// The Mods tab's Highway section's gallery and preview (launcher 3.30): this game has no highway mods.
inline constexpr bool kHighwayPreview = false;
inline constexpr HighwayGallery kHighwayGallery{};
inline constexpr HighwayAssets kHighwayAssets{};

// Whether some switch is in `group`.
constexpr bool HasToggles(ToggleGroup group) {
  for (const Toggle& toggle : kToggles) {
    if (toggle.group == group) return true;
  }
  return false;
}

// The Settings tab has a Game category (page_settings.cpp) when the game has a language row or switches of its own.
// Read by RecompLauncher before 3.6 only (from 3.6 the Game category is always there, with the launcher's language).
inline constexpr bool kHasGamePage = !kLanguages.empty() || HasToggles(ToggleGroup::kGame);

// --- Downloadable content ---

// The DLC page (page_dlc.cpp, dlc.h): Xbox 360 content packages, each installed by the game's --install_content tool
// mode (SDK change 44) into its user data's content folder. Off until checked with this game's content.
inline constexpr bool kHasDlcPage = false;

// NativeXRenderer's pipeline list (its D129, pipelines.h): Play first runs the game's --native_x_renderer_prepare_pipelines
// ("Compiling shaders") when the executable, the list (<TITLEID>.pipelines.list beside it) or the graphics driver
// changed since the last time. On when the game's NativeXRenderer profile names its shader files.
inline constexpr bool kHasPipelineList = true;

// --- The keyboard ---

// The Keyboard page's defaults where they are not the SDK's (its "Reset to defaults", and a control the files do not
// set): keybind_* cvars and their keys, as src/startup.cpp writes them for the game. The keyboard as a guitar (the SDK's
// keyboard_guitar_layout = "gh3"; GameKeyboardControlsFor): the frets D, F, J, K, L (A, B, Y, X, LB), green selecting and
// red going back in the menus, and in a song on the game's controller layout (a fret pressed plays its note); the
// arrows the D-pad (the menus); Space or Backspace Star Power (Back); Enter or X Start; the right arrow the whammy bar
// (the right stick); the triggers, RB and the sticks' other keys off, as W A S D, F and K would move them with the
// frets. Empty: the SDK's (game_types.h's KeyDefault).
inline constexpr std::array<KeyDefault, 23> kKeyboardDefaults{{
    {"keybind_left_trigger", ""},
    {"keybind_left_shoulder", "L"},
    {"keybind_right_shoulder", ""},
    {"keybind_right_trigger", ""},
    {"keybind_a", "D"},
    {"keybind_b", "F"},
    {"keybind_y", "J"},
    {"keybind_x", "K"},
    {"keybind_dpad_up", "Up"},
    {"keybind_dpad_down", "Down"},
    {"keybind_dpad_left", "Shift+Left"},
    {"keybind_dpad_right", "Shift+Right"},
    {"keybind_back", "Space,Backspace"},
    {"keybind_start", "Return,X"},
    {"keybind_rstick_right", "Right"},
    {"keybind_rstick_up", ""},
    {"keybind_rstick_down", ""},
    {"keybind_rstick_left", ""},
    {"keybind_rstick_press", ""},
    {"keybind_lstick_up", ""},
    {"keybind_lstick_down", ""},
    {"keybind_lstick_left", ""},
    {"keybind_lstick_right", ""},
}};
// The Keyboard page's switch that makes the keyboard's frets strum as they are pressed (the SDK's mnk_tap_target): none,
// a fret key pressed already plays its note in a song (the game's controller layout, keyboard_guitar_layout = "gh3").
inline constexpr const char* kKeyboardTapTarget = "";
// A real guitar's controls apart from a controller's (launcher 3.21.2): a guitar plays on its frets (A, B, Y, X, LB,
// GameGuitarControlsFor), a controller on the game's controller layout (GameControlsFor), the keyboard as a guitar in
// the menus and on that layout in a song (GameKeyboardControlsFor, launcher 3.33.2).
inline constexpr bool kGuitarControlsApart = true;
// The whammy bar's row on each controller playing guitar (page_controls.cpp, RecompLauncher 3.4): how far the game
// reads the bar, and its calibration (the SDK's gamepad_whammy_calibration, recomp change 60). False: no such row (a
// game without a guitar).
inline constexpr bool kHasWhammy = true;

// --- Singing ---

// The Controls tab's microphone and each controller's part (page_controls.cpp, RecompLauncher 3.1): a recording device
// of the PC as the game's USB microphone (the SDK's usb_microphone_1, recomp change 55) and a controller playing the
// game's instrument or singing (the SDK's gamepad_device_subtypes, change 56). False: the game has no microphone.
inline constexpr bool kHasMicrophones = false;
// The microphone's gain in dB (usb_microphone_1_gain) until the player changes it, and the Microphone's Reset.
inline constexpr int kMicrophoneGainDefault = 0;

// --- The controls in the game's words ---

// The game's functions (game_types.h's GameControl) in groups, in the launcher's language (game.cpp); empty: the pages show the Xbox 360 controls
// (texts.cpp). The Keyboard page lists after them, under "Other controls", the ones no function is on.
const std::vector<GameControlGroup>& GameControlsFor(uint32_t language);

// --- Texts of the game's own (game.cpp) ---

// --- Drums ---

// Guitar Hero III's engine has no drums: launcher 3.23.1's defaults (game_types.h: kHasDrums false, no drum keys or
// controls) leave out every drum row.

struct GameTexts {
  // The first run's welcome, in the game's spirit; null: the launcher's own (texts.cpp).
  const char* welcome_title;
  const char* welcome_text;
  // kToggles' rows: a title and a hint each.
  std::array<const char*, kToggles.size() * 2> toggles;
  // kToggles' help, the Settings tab's panel for the row in focus; null: the hint.
  std::array<const char*, kToggles.size()> toggle_help{};
  // kChoices' rows (launcher 3.29): a title and a hint each, their help (null: the hint) and their values' names (null
  // or missing: the value and the row's suffix).
  std::array<const char*, kChoices.size() * 2> choices{};
  std::array<const char*, kChoices.size()> choice_help{};
  std::array<std::array<const char*, 2>, kChoices.size()> choice_values{};
  // The first run's last screen (the game installed), in the game's spirit; null: the launcher's own.
  const char* done_title = nullptr;
  const char* done_text = nullptr;
  // The DLC page: where the game shows what was installed; null: nothing said.
  const char* dlc_where = nullptr;
  // The first run's extraction screen (while it runs), in the game's spirit; null: the launcher's own.
  const char* extracting_title = nullptr;
  const char* extracting_text = nullptr;
  // The Controls tab's keyboard switch for kKeyboardTapTarget, and its tip in the help.
  const char* keyboard_tap_title = nullptr;
  const char* keyboard_tap_hint = nullptr;
  // kHasMicrophones: a controller's two parts, the game's instrument and the singer's; null: the launcher's words.
  const char* part_instrument = nullptr;
  const char* part_vocals = nullptr;
};
// In the language of the launcher's texts (an XLanguage); missing ones fall back to English.
const GameTexts& GameTextsFor(uint32_t language);

}  // namespace launcher::game
