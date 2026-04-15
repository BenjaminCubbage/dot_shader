#pragma once
#include <atomic>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <windows.h>
#include "dot_shader/thread/mpmc_queue.h"
#include "window.h"

namespace DotShader::Window {

class WindowManager {
  public:
    static constexpr unsigned int QueueCapacity{ 2048 };

    enum class State {
        Started,
        Starting,
        Stopping,
        Stopped
    };

    enum class Result {
        Success = 0,
        NotStopped,
        NotStarted,
        ThreadQueueFull
    };

    WindowManager() = delete;

    static Result start();
    static Result stop();
    static Result open_window();

    static inline State state() {
        return m_state;
    }

  private:
    struct ThreadMessage {
        enum class Type { 
            OpenWindow,
            Stop
        };

        Type type;
    };
    
    using ThreadQueue = Thread::MPMCQueue<ThreadMessage, QueueCapacity>;

    static void thread_loop(void*);

    /*
        This is not accessed by the window thread.
    */
    static inline std::atomic<State>          m_state{ State::Stopped };
    static inline std::optional<std::jthread> m_thread{ std::nullopt };
    
    /*
        This is shared with the window thread.
    */
    static inline std::optional<HANDLE> m_signal{ std::nullopt };
    static inline std::shared_mutex     m_queue_mutex{};
    static inline ThreadQueue           m_thread_queue{};

    /*
        This is owned by the window thread.
    */
    static inline std::vector<Window> m_windows{};
};

}