#pragma once
#include "render_surface.h"
#include <optional>

namespace DotShader::Render {

class Renderer {
  public:
    static void initialize(HWND hwnd);

  private:
    static inline std::optional<RenderSurface> m_surface{ std::nullopt };
};

}