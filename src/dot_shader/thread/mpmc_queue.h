#pragma once
#include <array>
#include <atomic>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace DotShader::Thread
{

/*
    This is based on a bounded MPMC algorithm by Dmitry Vyukov.
*/
template<typename T, size_t Capacity>
class MPMCQueue
{
  private:
    static_assert(
        (Capacity > 1) && !(Capacity & (Capacity - 1)), 
        "Capacity must be a power of two.");

    static_assert(
        std::is_default_constructible_v<T>,
        "T must be default constructible");

    static_assert(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_move_assignable_v<T>,
        "T must be nothrow moveable");

    struct Cell
    {
        std::atomic<size_t> seq;
        T data;
    };

  public:
    struct EnqTicket
    {
        size_t t;
        Cell* cell;
    };

    MPMCQueue           (const MPMCQueue&) = delete;
    MPMCQueue& operator=(const MPMCQueue&) = delete;
    MPMCQueue           (MPMCQueue&&) = delete;
    MPMCQueue& operator=(MPMCQueue&&) = delete;

    MPMCQueue()
    {
        reset();
    }

    /*
        Reset the queue. 
        
        This function is not thread-safe.
    */
    void reset() noexcept {
        m_head = 0;
        m_tail = 0;

        for (size_t i = 0; i < m_jobs.size(); ++i) {
            m_jobs[i].seq  = i;
            m_jobs[i].data = {};
        }
    }

    /*
        Try to dequeue an item. Returns std::nullopt if
        nothing found.
    */
    std::optional<T>
    try_dequeue() noexcept
    {
        for (;;)
        {
            size_t h = m_head.load(std::memory_order_relaxed);
            Cell& job = m_jobs[h & (Capacity - 1)];

            size_t seq = job.seq.load(std::memory_order_acquire);

            if (seq <= h)
                return std::nullopt;

            if (seq > h + 1)
                continue;

            if (!m_head.compare_exchange_strong(
                    h, h + 1,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed))
                continue;

            auto data = std::move(job.data);
            job.seq.store(h + Capacity, std::memory_order_release);
            return std::move(data);
        }
    }

    /*
        Reserve a slot in the queue, but don't mark it as "ready"
        until enqueue_publish() is called.

        Separating enqueue_reserve and enqueue_publish allows the
        caller to:

        (1) Reserve a queue cell (ensuring capacity exists) without
            making it visible to consumers yet, then
        (2) perform any prerequisite work (e.g., reserving a
            completion slot) and fully initialize the cell, and finally
        (3) publish the cell with a release-store so consumers may
            digest it.

        Once a slot is reserved, it should be committed quickly; the
        head cannot move beyond an uncommitted reservation, and the
        queue will eventually halt on the uncommitted enqueue.

        Thus, only light, noexcept work--or none at all--should be performed
        between reservation and committing.
    */
    bool try_enqueue_acquire(EnqTicket* out_ticket) noexcept
    {
        for (;;)
        {
            size_t t = m_tail.load(std::memory_order_relaxed);
            Cell& job = m_jobs[t & (Capacity - 1)];

            size_t seq = job.seq.load(std::memory_order_acquire);

            if (seq < t)
                return false;

            if (seq > t)
                continue;

            if (!m_tail.compare_exchange_strong(
                t, t + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
                continue;
            }

            *out_ticket = {
                .t    = t,
                .cell = &job
            };

            return true;
        }
    }

    /*
        Mark the acquired ticket as "ready".
    */
    void enqueue_publish(EnqTicket ticket, T&& data) noexcept
    {
        ticket.cell->data = std::move(data);
        ticket.cell->seq.store(ticket.t + 1, std::memory_order_release);
    }

  private:
    alignas(64) std::atomic<size_t> m_head{ 0 };
    alignas(64) std::atomic<size_t> m_tail{ 0 };
    std::array<Cell, Capacity> m_jobs;
};

}