#pragma once
#include <unordered_map>
#include <vector>

namespace DotShader::Window {

class WindowInst;

enum class WindowEventType {
    Created,
    CreationFailed,
    MouseMove,
    Closed
};

union WindowEvent {
    struct {
        WindowInst* window_inst;
    } created;

    struct {
        WindowInst* window_inst;
        int x;
        int y;
    } mouse_move;
};

class WindowEventHandlers {
  public:
    using HandlerMap = std::unordered_map<
        WindowEventType,
        std::vector<void (*)(const WindowEvent&, void*, void*)>>;

    WindowEventHandlers(
        void* context1,
        void* context2,
        HandlerMap handlers)
        : m_context1(context1)
        , m_context2(context2)
        , m_handlers(std::move(handlers)) {}

    WindowEventHandlers           (const WindowEventHandlers&) = delete;
    WindowEventHandlers& operator=(const WindowEventHandlers&) = delete;
    WindowEventHandlers           (WindowEventHandlers&&) noexcept = default;
    WindowEventHandlers& operator=(WindowEventHandlers&&) noexcept = default;

    void call(WindowEventType type, const WindowEvent& event);

  private:
    void* m_context1;
    void* m_context2;

    HandlerMap m_handlers;
};

}