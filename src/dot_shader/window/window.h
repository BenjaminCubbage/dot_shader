#pragma once
#include <windows.h>

namespace DotShader::Window {
    
class WindowInst;

class IWindow {
  public:
    IWindow() {}
    ~IWindow() {}

    IWindow           (const IWindow&) = delete;
    IWindow& operator=(const IWindow&) = delete;
    IWindow           (IWindow&&) noexcept = default;
    IWindow& operator=(IWindow&&) noexcept = default;

    virtual const char* window_title() { return "Window"; }

    virtual void on_created(WindowInst* window_inst) {}
    virtual void on_creation_failed() {}
    virtual void on_mouse_move(WindowInst* window_inst, int x, int y) {}
    virtual void on_closed() {}
};

}