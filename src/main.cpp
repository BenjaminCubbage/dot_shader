#include <iostream>
#include <thread>
#include <chrono>
#include <format>
#include "dot_shader/window/window_event.h"
#include "dot_shader/window/window_manager.h"
#include "dot_shader/window/window.h"

using namespace DotShader::Window;

void on_window_created(const WindowEvent& event, void* c1, void* c2) {
    std::cout
        << "Window opened"
        << std::endl;
}

void on_mouse_move(const WindowEvent& event, void* c1, void* c2) {
    std::cout
        << std::format(
            "[{:<10}, {:<10}]",
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
                { WindowEventType::Created,   { &on_window_created } },
                { WindowEventType::MouseMove, { &on_mouse_move } }
            }));

    while (1){}
    WindowManager::stop();
}