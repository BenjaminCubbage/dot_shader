#include "window.h"
#include <iostream>

namespace DotShader::Window {

ATOM Window::window_class() {
    constexpr const char* ClassName = "WINDOW_CLASS";
    static ATOM atom{};
    if (atom == 0) {
        WNDCLASSA wc{};
        wc.lpfnWndProc   = &Window::window_proc;
        wc.hInstance     = hinstance();
        wc.lpszClassName = ClassName;

        atom = RegisterClassA(&wc);
        if (atom == 0)
            std::cout
                << "RegisterClass failed: "
                << GetLastError()
                << std::endl;
    }
    return atom;
}

Window::Window() {
    hwnd = CreateWindowExA(
        0,
        MAKEINTATOM(window_class()),
        "Dot Shader",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hinstance(),
        NULL);
    if (hwnd == 0) {
        std::cout
            << "CreateWindow failed: "
            << GetLastError()
            << std::endl;
        return;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
}

Window::~Window() {
    if (hwnd != 0)
        DestroyWindow(hwnd);
}

Window::Window(Window&& other) noexcept {
    other.hwnd = hwnd;
    hwnd       = 0;
}

LRESULT Window::window_proc(
    HWND hwnd,
    unsigned int umsg,
    WPARAM w_param,
    LPARAM l_param) {
    return DefWindowProc(hwnd, umsg, w_param, l_param);
}

}