#include <atomic>
#include <iostream>
#include <format>
#include "dot_shader/window/window_event.h"
#include "dot_shader/window/window_manager.h"
#include "dot_shader/window/window.h"

using namespace DotShader::Window;

std::atomic_flag main_window_closed;

void on_window_created(const WindowEvent& event, void* c1, void* c2) {
}

void on_window_creation_failed(const WindowEvent&, void*, void*) {
    std::cout << "Window creation failed" << std::endl;

    main_window_closed.test_and_set(
        std::memory_order_release);
    main_window_closed.notify_all();
}

void on_window_closed(const WindowEvent& event, void* c1, void* c2) {
    main_window_closed.test_and_set(
        std::memory_order_release);
    main_window_closed.notify_all();
}

void on_mouse_move(const WindowEvent& event, void* c1, void* c2) {
    std::cout
        << std::format(
            "[{:>4}, {:<4}]",
            event.mouse_move.x,
            event.mouse_move.y)
        << std::endl;
}

int main() {
    WindowManager::start();
    WindowManager::open_window(
        WindowEventHandlers(
            reinterpret_cast<void*>(12),
            reinterpret_cast<void*>(15), {
                { WindowEventType::Created,        { &on_window_created } },
                { WindowEventType::CreationFailed, { &on_window_creation_failed } },
                { WindowEventType::Closed,         { &on_window_closed } },
                { WindowEventType::MouseMove,      { &on_mouse_move } }
            }));

    main_window_closed.wait(
        false,
        std::memory_order_acquire);
    WindowManager::stop();
}