// ghaerosmith - the note highway's picture from the player's folder of images: one chosen, or a random one at each
// song, each darkened toward black as the player asks (the launcher's Mods > Highway gallery). GH5's (its
// src/highway_images.cpp, from GH Warriors of Rock). NativeXRenderer draws the image instead of the highway's texture
// through its D177 ReplaceTexture: decoded on a worker thread, so the next random one is preloaded while a song plays
// and shown at the next song's start. Nothing without NativeXRenderer (the Xenos path draws the game's own); the folder
// of images named by hash (D176) stays for any texture, and this choice wins over it.
//
// Guitar Hero III's engine gives each guitarist a highway of his own (setup_highway's sprite takes the material of
// player_status.highway_material): models/guitarists/<name>_highway.tex in the guitarist's packs
// (DATA/COMPRESSED/PAK/<name>.pak.xen, one raw deflate stream; each outfit's color pack carries it again), 14 of them,
// each a tiled 512x1024 DXT1 with 9 levels. The picture is asked for on all 14 (NativeXRenderer's D183 shares one texture
// among them). Their hashes (NativeXRenderer's base_hash: XXH3_64 of the base level as the game loaded it, 262144
// bytes) were computed from the disc's dictionaries (the data at the entry's offset, +0x1000 for one texture), the same
// in every pack that holds one; native_x_renderer_texture_log = true confirmed Casey's (62065606BEADBB22, k_DXT1
// 512x1024, 9 levels) in a song. A song's start: setup_highway (guitar_highway.qb, from start_gem_scroller, once per
// player, no wait in it) evaluates its first expressions (src/debug_menu.cpp's hooks): the first after 3 s without one
// is a new song.

#include "highway_images.h"

#include <native_x_renderer/native_x_renderer.h>
#include <rex/cvar.h>
#include <rex/logging.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <random>
#include <string>
#include <vector>

REXCVAR_DEFINE_STRING(ghaerosmith_highway_folder, "", "GHAerosmith",
                      "The folder of the note highway's pictures (the launcher's gallery): PNG, JPEG, BMP or TGA, any "
                      "size (stretched over the highway). Empty: none");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_image, "", "GHAerosmith",
                      "The note highway's picture: a file of ghaerosmith_highway_folder, \"<random>\" for a random one at "
                      "each song, or empty for the game's own");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_random_from, "all", "GHAerosmith",
                      "Where a random highway picture comes from: \"all\" the folder's, or \"favorites\" "
                      "(ghaerosmith_highway_favorites; all when none of them is there)");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_favorites, "", "GHAerosmith",
                      "The favorite highway pictures, file names separated by |");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_dim, "off", "GHAerosmith",
                      "Bright highway pictures darkened toward black, for every picture: \"off\", \"auto\" (only as much "
                      "as a picture is bright) or a brightness in percent (90-40)");
REXCVAR_DEFINE_STRING(ghaerosmith_highway_image_dims, "", "GHAerosmith",
                      "Pictures with a darkening of their own: name:value separated by | (value as ghaerosmith_highway_dim)");

namespace {

// Each guitarist's highway texture, NativeXRenderer's D176 hash (from the disc).
constexpr uint64_t kHighwayTextures[] = {
    0xB311F0858344B883,  // axel_1_highway
    0x47EB5BF24CF78FBE,  // bradw_aero_1_highway (Brad Whitford)
    0x62065606BEADBB22,  // casey_1_highway
    0xCFEAB1F5B37D5BD9,  // dmc_1_highway
    0xF82BAFB98E44ABC7,  // izzy_1_highway
    0x3452F9C82296C39C,  // joep_1_highway (Joe Perry)
    0xCCF1FC7F01AC2C08,  // johnny_1_highway
    0x22FE5CF3AC2E7F39,  // judy_1_highway
    0xDE7B142C4655052A,  // lars_1_highway
    0x69A82E417E3A9BEB,  // midori_1_highway
    0x7C1C42DCF5CF1AA2,  // ripper_1_highway
    0xE1912705ABB6BB22,  // satan_1_highway
    0xC27B0C8F71EA82C0,  // tomh_aero_1_highway (Tom Hamilton)
    0x28FDF66EEF08F381,  // xavier_1_highway
};
constexpr const char* kRandom = "<random>";

std::vector<std::string> Split(const std::string& s, char separator) {
  std::vector<std::string> out;
  size_t start = 0;
  while (start <= s.size()) {
    size_t end = s.find(separator, start);
    if (end == std::string::npos) end = s.size();
    if (end > start) out.push_back(s.substr(start, end - start));
    start = end + 1;
  }
  return out;
}

std::string Lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
  return s;
}

// "off" 1, "auto" NativeXRenderer's automatic, a percent its fraction.
float Brightness(const std::string& value) {
  if (value == "auto") return native_x_renderer::kAutoBrightness;
  if (value.empty() || value == "off") return 1.0f;
  return std::clamp(float(std::atoi(value.c_str())) / 100.0f, 0.1f, 1.0f);
}

struct State {
  std::mutex mutex;
  std::filesystem::path folder;
  std::vector<std::string> pool;  // the file names a random picture comes from
  std::vector<std::pair<std::string, std::string>> dims;
  std::string global_dim;
  bool random = false;
  std::string current, next;
  std::mt19937 rng{uint32_t(std::chrono::steady_clock::now().time_since_epoch().count())};
  std::chrono::steady_clock::time_point last_song{};
  bool active = false;
};

State& TheState() {
  static State state;
  return state;
}

float BrightnessOf(const State& state, const std::string& name) {
  for (const auto& [file, value] : state.dims) {
    if (Lower(file) == Lower(name)) return Brightness(value);
  }
  return Brightness(state.global_dim);
}

std::string PickLocked(State& state, const std::string& not_this) {
  if (state.pool.empty()) return {};
  if (state.pool.size() == 1) return state.pool.front();
  std::uniform_int_distribution<size_t> pick(0, state.pool.size() - 1);
  for (int tries = 0; tries < 8; ++tries) {
    const std::string& name = state.pool[pick(state.rng)];
    if (name != not_this) return name;
  }
  return state.pool.front() != not_this ? state.pool.front() : state.pool.back();
}

void ShowLocked(State& state, const std::string& name) {
  state.current = name;
  const float brightness = BrightnessOf(state, name);
  for (uint64_t texture : kHighwayTextures) {
    native_x_renderer::ReplaceTexture(texture, state.folder / std::filesystem::u8path(name), brightness);
  }
  REXLOG_INFO("[mods] the highway's picture: {} ({})", name,
              brightness < 0 ? std::string("darkened automatically")
              : brightness < 1.0f ? fmt::format("darkened to {:.0f} %", brightness * 100.0f)
                                  : std::string("as it is"));
}

}  // namespace

namespace ghaerosmith::highway {

void Configure() {
  State& state = TheState();
  std::lock_guard lock(state.mutex);
  const std::string image = REXCVAR_GET(ghaerosmith_highway_image);
  const std::string folder = REXCVAR_GET(ghaerosmith_highway_folder);
  if (image.empty() || folder.empty()) return;
  state.folder = std::filesystem::u8path(folder);
  state.global_dim = Lower(REXCVAR_GET(ghaerosmith_highway_dim));
  for (const std::string& item : Split(REXCVAR_GET(ghaerosmith_highway_image_dims), '|')) {
    const size_t colon = item.rfind(':');
    if (colon != std::string::npos) state.dims.emplace_back(item.substr(0, colon), Lower(item.substr(colon + 1)));
  }
  std::error_code error;
  if (!std::filesystem::is_directory(state.folder, error)) {
    REXLOG_WARN("[mods] the highway's pictures: no folder {}", folder);
    return;
  }
  if (image != kRandom) {
    if (!std::filesystem::is_regular_file(state.folder / std::filesystem::u8path(image), error)) {
      REXLOG_WARN("[mods] the highway's picture {} is not in {}", image, folder);
      return;
    }
    state.active = true;
    ShowLocked(state, image);
    return;
  }
  // A random one: the folder's pictures, or the favorites there.
  std::vector<std::string> all;
  for (const auto& entry : std::filesystem::directory_iterator(state.folder, error)) {
    const std::string ext = Lower(entry.path().extension().string());
    if (entry.is_regular_file(error) &&
        (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")) {
      const std::u8string name = entry.path().filename().u8string();
      all.emplace_back(reinterpret_cast<const char*>(name.data()), name.size());
    }
  }
  std::sort(all.begin(), all.end());
  if (Lower(REXCVAR_GET(ghaerosmith_highway_random_from)) == "favorites") {
    for (const std::string& favorite : Split(REXCVAR_GET(ghaerosmith_highway_favorites), '|')) {
      auto it = std::find_if(all.begin(), all.end(), [&](const std::string& n) { return Lower(n) == Lower(favorite); });
      if (it != all.end()) state.pool.push_back(*it);
    }
    if (state.pool.empty()) REXLOG_INFO("[mods] the highway's pictures: no favorite in the folder, any picture");
  }
  if (state.pool.empty()) state.pool = all;
  if (state.pool.empty()) {
    REXLOG_WARN("[mods] the highway's pictures: none in {}", folder);
    return;
  }
  state.random = true;
  state.active = true;
  REXLOG_INFO("[mods] the highway's picture: a random one of {} at each song", state.pool.size());
  ShowLocked(state, PickLocked(state, {}));
  state.next = PickLocked(state, state.current);
  native_x_renderer::PreloadTextureImage(state.folder / std::filesystem::u8path(state.next),
                                         BrightnessOf(state, state.next));
}

bool Random() {
  State& state = TheState();
  std::lock_guard lock(state.mutex);
  return state.random;
}

}  // namespace ghaerosmith::highway

// From src/debug_menu.cpp's evaluation hook, when setup_highway evaluates an expression (a highway made, at a song's
// start): a random picture changes at each song, the next one preloaded.
void ghaerosmith::highway::OnHighwaySetup() {
  State& state = TheState();
  if (!state.random) return;
  std::lock_guard lock(state.mutex);
  const auto now = std::chrono::steady_clock::now();
  const bool new_song = now - state.last_song > std::chrono::seconds(3);
  state.last_song = now;
  if (!new_song || state.next.empty()) return;
  ShowLocked(state, state.next);
  state.next = PickLocked(state, state.current);
  native_x_renderer::PreloadTextureImage(state.folder / std::filesystem::u8path(state.next),
                                         BrightnessOf(state, state.next));
}
