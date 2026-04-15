#pragma once
#include <windows.h>

namespace DotShader::Window {

/*
    This class is designed to be used by the WindowManager.

    Do not destruct a Window that was constructed on a different
    thread.
*/
class Window {
  public:
    Window();
    ~Window();
    Window(Window&& other) noexcept;

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

  private:
    static LRESULT window_proc(
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

    HWND hwnd{ 0 };
};

}