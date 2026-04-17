#pragma once
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <windows.h>
#include "dot_shader/thread/mpmc_queue.h"
#include "window.h"
#include "window_inst.h"

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
        ThreadQueueFull,
        CannotJoinToUIThread
    };

    WindowManager() = delete;

    static Result start();
    static Result stop();
    static Result open_window(std::unique_ptr<IWindow> window);
    
    template<class T>
        requires std::is_base_of_v<IWindow, T>
    static inline Result open_window() {
        return open_window(std::make_unique<T>());
    }

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
        std::optional<std::unique_ptr<IWindow>> window{ std::nullopt };
    };
    
    static void cleanup_destroyed_inst(WindowInst* window_inst);
    
    using ThreadQueue = Thread::MPMCQueue<ThreadMessage, QueueCapacity>;

    static void thread_loop(void*);

    /*
        This is not accessed by the UI thread.
    */
    static inline std::optional<std::jthread> m_thread{ std::nullopt };
    static inline std::atomic<State>          m_state{ State::Stopped };
    
    /*
        This is shared with the UI thread.
    */
    static inline std::optional<HANDLE> m_signal{ std::nullopt };
    static inline std::shared_mutex     m_queue_mutex{};
    static inline ThreadQueue           m_thread_queue{};

    /*
        This is owned by the UI thread.
    */
    static inline std::list<WindowInst> m_window_insts{};
};

}