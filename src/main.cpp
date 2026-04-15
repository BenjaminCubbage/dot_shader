#include <iostream>
#include <thread>
#include <chrono>
#include "dot_shader/window/window.h"
#include "dot_shader/window/window_manager.h"

int main() {
    DotShader::Window::WindowManager::start();
    DotShader::Window::WindowManager::open_window();
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    DotShader::Window::WindowManager::stop();
}