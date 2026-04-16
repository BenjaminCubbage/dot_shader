#pragma once
#include <functional>
#include <windows.h>
#include "window_event.h"

namespace DotShader::Window {

/*
    This class is designed to be used by the WindowManager.

    It should only be created, destructed, and accessed on the
    UI thread.
*/
class Window {
  public:
    Window(WindowEventHandlers handlers);

    ~Window();
    Window(Window&& other) noexcept;

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

  private:
    static LRESULT CALLBACK window_proc(
        HWND hwnd,
        unsigned int umsg,
        WPARAM w_param,
        LPARAM l_param);

    static ATOM window_class();

    static inline HINSTANCE hinstance() {
        static HINSTANCE hinstance{};
        if (!hinstance)
            hinstance = GetModuleHandleA(0);
        return hinstance;
    }

    HWND m_hwnd{ 0 };
    WindowEventHandlers m_handlers;
};

}