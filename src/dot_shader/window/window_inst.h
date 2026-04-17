#pragma once
#include <functional>
#include <memory>
#include <windows.h>
#include "window.h"

namespace DotShader::Window {

/*
    This class is designed to be used by the WindowManager.

    **It should only be created, destructed, and accessed on the
    UI thread.**
    
    **Its location in memory should be stable
*/
class WindowInst {
  public:
    WindowInst(
        std::unique_ptr<IWindow> window,
        void (*destroy)(WindowInst* window_inst));

    ~WindowInst();

    WindowInst           (const WindowInst&) = delete;
    WindowInst& operator=(const WindowInst&) = delete;

    WindowInst           (WindowInst&&) = delete;
    WindowInst& operator=(WindowInst&&) = delete;

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
    std::unique_ptr<IWindow> m_window;
    void (*m_destroy)(WindowInst* window_inst);
};

}