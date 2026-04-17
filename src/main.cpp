#include <atomic>
#include <iostream>
#include <format>
#include <memory>
#include "dot_shader/window/window.h"
#include "dot_shader/window/window_manager.h"
#include "dot_shader/window/window_inst.h"

using namespace DotShader::Window;

std::atomic_flag main_window_closed;

class MainWindow final : public IWindow {
    const char* window_title() override {
        return "Dot Shader!";
    }

    void on_created(WindowInst* window_inst) override {
        std::cout << "created main" << std::endl;
    }

    void on_mouse_move(WindowInst* window_inst, int x, int y) override {
    }

    void on_creation_failed() override {
        main_window_closed.test_and_set(std::memory_order_release);
        main_window_closed.notify_all();
    }

    void on_closed() override {
        main_window_closed.test_and_set(std::memory_order_release);
        main_window_closed.notify_all();
    }
};

int main() {
    WindowManager::start();
    WindowManager::open_window<MainWindow>();

    main_window_closed.wait(
        false,
        std::memory_order_acquire);
    WindowManager::stop();
}