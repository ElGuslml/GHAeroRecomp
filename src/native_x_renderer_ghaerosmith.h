// ghaerosmith - NativeXRenderer for Guitar Hero: Aerosmith (see native_x_renderer_ghaerosmith.cpp).

#pragma once

#include <native_x_renderer/native_x_renderer.h>

namespace ghaerosmith {

// Guitar Hero: Aerosmith's D3D library: the XDK device and object layouts (the XEX imports 2.0.6683, the D3D
// library itself reports 6534).
const native_x_renderer::Profile& NativeXRendererProfile();

}  // namespace ghaerosmith
