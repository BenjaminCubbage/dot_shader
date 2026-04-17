#include "window.h"
#include <iostream>
#include <stdexcept>
#include <utility>
#include <format>

namespace DotShader::Window {

Window::Window(WindowEventHandlers handlers)
    : m_handlers(std::move(handlers)) {
    m_hwnd = CreateWindowExA(
        0,
        MAKEINTATOM(window_class()),
        "Dot Shader",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hinstance(),
        nullptr);
    if (m_hwnd == 0) {
        m_handlers.call(WindowEventType::CreationFailed, {});
        throw std::exception(
            std::format("Error creating window: {}", GetLastError()).c_str());
    }

    SetLastError(0);
    const LONG_PTR ptr = SetWindowLongPtrA(
        m_hwnd,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    if (ptr == 0 && GetLastError() != 0) {
        m_handlers.call(WindowEventType::CreationFailed, {});
        throw std::exception(
            std::format("Error setting window data: {}", GetLastError()).c_str());
    }

    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    m_handlers.call(WindowEventType::Created, WindowEvent{
        .created = { .window = this }
    });
}

Window::~Window() {
    if (m_hwnd != 0)
        DestroyWindow(m_hwnd);
}

Window::Window(Window&& other) noexcept
    : m_hwnd(std::exchange(other.m_hwnd, HWND{ 0 }))
    , m_handlers(std::move(other.m_handlers)) {}

/*
    This function will not throw. Instead, if
    registering the class initially fails, it
    will return a zero ATOM and retry on
    subsequent attempts.
*/
ATOM Window::window_class() {
    static ATOM atom{};
    if (atom == 0) {
        WNDCLASSA wc{};
        wc.hInstance     = hinstance();
        wc.lpfnWndProc   = &Window::window_proc;
        wc.lpszClassName = "dotshader";

        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));

        atom = RegisterClassA(&wc);
        if (atom == 0)
            std::cout
                << "Could not register class: "
                << GetLastError()
                << std::endl;
    }
    return atom;
}

LRESULT Window::window_proc(
    HWND hwnd,
    unsigned int umsg,
    WPARAM w_param,
    LPARAM l_param) {
    LONG_PTR window_data = GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (window_data == 0)
        return DefWindowProc(hwnd, umsg, w_param, l_param);

    Window* window =
        reinterpret_cast<Window*>(window_data);

    switch (umsg) {
    case WM_MOUSEMOVE: {
        const int x = ((l_param & 0x0000FFFF) >>  0);
        const int y = ((l_param & 0xFFFF0000) >> 16);
        window->m_handlers.call(WindowEventType::MouseMove, WindowEvent{
            .mouse_move = {
                .window = window,
                .x      = x,
                .y      = y
            }
        });
        break;
    }

    case WM_NCDESTROY:
        window->m_handlers.call(WindowEventType::Closed, {});
        break;
    }

    return DefWindowProc(hwnd, umsg, w_param, l_param);
}

}