#include "Renderer.h"

namespace DotShader::Render {

void Renderer::initialize(HWND hwnd) {
    m_surface.emplace(hwnd);
}

}