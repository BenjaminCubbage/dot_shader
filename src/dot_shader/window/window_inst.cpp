#include "window_inst.h"
#include <iostream>
#include <stdexcept>
#include <utility>
#include <format>

namespace DotShader::Window {

WindowInst::WindowInst(
    std::unique_ptr<IWindow> window,
    void (*destroy)(WindowInst* window_inst))
    : m_window(std::move(window))
    , m_destroy(destroy) {
    m_hwnd = CreateWindowExA(
        0,
        MAKEINTATOM(window_class()),
        m_window->window_title(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hinstance(),
        nullptr);
    if (m_hwnd == 0) {
        m_window->on_creation_failed();
        throw std::exception(
            std::format("Error creating window: {}", GetLastError()).c_str());
    }

    SetLastError(0);
    const LONG_PTR ptr = SetWindowLongPtrA(
        m_hwnd,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    if (ptr == 0 && GetLastError() != 0) {
        m_window->on_creation_failed();
        throw std::exception(
            std::format("Error setting window data: {}", GetLastError()).c_str());
    }

    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    m_window->on_created(this);
}

WindowInst::~WindowInst() {
    DestroyWindow(m_hwnd);
}

LRESULT WindowInst::window_proc(
    HWND hwnd,
    unsigned int umsg,
    WPARAM w_param,
    LPARAM l_param) {
    LONG_PTR window_data = GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (window_data == 0)
        return DefWindowProc(hwnd, umsg, w_param, l_param);

    WindowInst* window_inst =
        reinterpret_cast<WindowInst*>(window_data);

    switch (umsg) {
    case WM_MOUSEMOVE: {
        const int x = ((l_param & 0x0000FFFF) >>  0);
        const int y = ((l_param & 0xFFFF0000) >> 16);
        window_inst->m_window->on_mouse_move(window_inst, x, y);
        break;
    }

    case WM_SIZE: {
        const int w = ((l_param & 0x0000FFFF) >>  0);
        const int h = ((l_param & 0xFFFF0000) >> 16);
        window_inst->m_window->on_resize(window_inst, w, h);
        break;
    }

    case WM_NCDESTROY:
        /*
            Destructor has called DestroyWindow.
        */
        window_inst->m_window->on_closed();
        break;
        
    case WM_CLOSE:
        /*
            Don't call into the DefWindowProc when we click
            the close button. That would destroy the window before
            the destructor has a chance to fire, which isn't
            what we want.
        */
        window_inst->m_destroy(window_inst);
        return 0;
    }

    return DefWindowProc(hwnd, umsg, w_param, l_param);
}

/*
    This function will not throw. Instead, if
    registering the class initially fails, it
    will return a zero ATOM and retry on
    subsequent attempts.
*/
ATOM WindowInst::window_class() {
    static ATOM atom{};
    if (atom == 0) {
        WNDCLASSA wc{};
        wc.hInstance     = hinstance();
        wc.lpfnWndProc   = &WindowInst::window_proc;
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

}