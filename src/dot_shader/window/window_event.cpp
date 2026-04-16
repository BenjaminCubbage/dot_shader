#include "window_event.h"
#include "window.h"

namespace DotShader::Window {

void WindowEventHandlers::call(WindowEventType type, const WindowEvent& event) {
    auto handler_vec = m_handlers.find(type);

    if (handler_vec == m_handlers.end())
        return;

    for (auto& handler : handler_vec->second)
        handler(event, m_context1, m_context2);
}

}