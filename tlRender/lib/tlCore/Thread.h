#include <chrono>
#include <future>
#include <thread>

namespace tl
{
    namespace thread
    {

// Join a std::thread, but give up (rather than block forever) if it hasn't
// exited within `timeout`. Returns true if it exited in time.
//
// std::thread has no wait-with-timeout of its own, so the actual (blocking)
// join is handed off to a detached watcher thread, and we time out on a
// future instead. `t` is moved into the watcher: from that point on the
// watcher alone owns and will join the underlying OS thread, whenever it
// eventually finishes. `t` itself is left moved-from (not joinable), so the
// caller's destructor for it is safe either way.
//
// NOTE: deliberately NOT built on std::async — a future returned by
// std::async blocks in its own destructor until the task finishes, which
// would silently reintroduce the exact hang this function exists to avoid.
//
// Usage:
// if (!joinWithTimeout(p.thread.thread, std::chrono::seconds(5)))
// {
//     std::cerr << "did not exit cleanly" << std::endl;
// }

        template <class Rep, class Period>
        bool joinWithTimeout(
            std::thread& t,
            const std::chrono::duration<Rep, Period>& timeout)
        {
            if (!t.joinable())
                return true;

            auto donePromise = std::make_shared<std::promise<void>>();
            std::future<void> doneFuture = donePromise->get_future();

            std::thread watcher(
                [thread = std::move(t), donePromise]() mutable
                    {
                        thread.join();
                        donePromise->set_value();
                    });
            watcher.detach();

            return doneFuture.wait_for(timeout) == std::future_status::ready;
        }

    }
}
