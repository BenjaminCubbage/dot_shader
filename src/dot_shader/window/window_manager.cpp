#include "window_manager.h"
#include "window_event.h"
#include <assert.h>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <windows.h>

namespace DotShader::Window {

WindowManager::Result WindowManager::start() {
    State swap{ State::Stopped };
    if (!m_state.compare_exchange_strong(
        swap,
        State::Starting,
        std::memory_order_acquire,
        std::memory_order_relaxed)) {
        return Result::NotStopped;
    }

    m_signal.emplace(CreateEventA(
        nullptr, 0, QueueCapacity, nullptr));
    m_thread.emplace(&thread_loop, nullptr);

    m_state.store(
        State::Started,
        std::memory_order_release);
    return Result::Success;
}

WindowManager::Result WindowManager::stop() {
    State swap{ State::Started };
    if (!m_state.compare_exchange_strong(
        swap,
        State::Stopping,
        std::memory_order_acquire,
        std::memory_order_relaxed)) {
        return Result::NotStarted;
    }

    ThreadQueue::EnqTicket ticket;
    if (!m_thread_queue.try_enqueue_acquire(&ticket)) {
        m_state.store(
            State::Started,
            std::memory_order_relaxed);
        return Result::ThreadQueueFull;
    }
    m_thread_queue.enqueue_publish(ticket, ThreadMessage{
        .type = ThreadMessage::Type::Stop
    });

    assert(m_signal.has_value());
    assert(m_thread.has_value());
    SetEvent(*m_signal);
    m_thread->join();
    m_thread.reset();
    m_signal.reset();

    {
        std::unique_lock lk(m_queue_mutex);
        m_thread_queue.reset();
    }

    m_state.store(
        State::Stopped,
        std::memory_order_release);
    return Result::Success;
}

WindowManager::Result WindowManager::open_window(
    WindowEventHandlers handlers) {
    if (m_state.load(std::memory_order_relaxed) != State::Started)
        return Result::NotStarted;

    {
        std::shared_lock lk(m_queue_mutex);
        if (m_state.load(std::memory_order_acquire) != State::Started)
            return Result::NotStarted;

        ThreadQueue::EnqTicket ticket;
        if (!m_thread_queue.try_enqueue_acquire(&ticket))
            return Result::ThreadQueueFull;

        m_thread_queue.enqueue_publish(ticket, ThreadMessage{
            .type     = ThreadMessage::Type::OpenWindow,
            .handlers = std::move(handlers)
        });
    }

    assert(m_signal.has_value());
    SetEvent(*m_signal);

    return Result::Success;
}

void WindowManager::thread_loop(void*) {
    assert(m_signal.has_value());

    while (true) {
        const int message = MsgWaitForMultipleObjectsEx(
            1,
            &*m_signal,
            INFINITE,
            QS_ALLINPUT,
            MWMO_INPUTAVAILABLE);

        MSG msg{};
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        /*
            A shared lock is not necessary here even though we are accessing
            the thread queue; the only time the thread queue is accessed in 
            an unsharable manner is when this thread has already been joined.
        */
        while (auto message = m_thread_queue.try_dequeue()) {
            switch (message->type) {
            case ThreadMessage::Type::OpenWindow: {
                try {
                    m_windows.emplace_back(
                        std::move(*message->handlers));
                } catch (std::exception e) {
                    std::cout << e.what() << std::endl;
                }
                break;
            }

            case ThreadMessage::Type::Stop:
                m_windows.clear();
                return;
            }
        }
    }
}

}