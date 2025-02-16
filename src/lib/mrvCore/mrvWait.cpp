
#include <chrono>

#include <FL/Fl.H>

namespace mrv
{
    namespace wait
    {
        void milliseconds(float duration)
        {
            const auto& start = std::chrono::high_resolution_clock::now();
            auto elapsedTime =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    start - start)
                    .count();
            while (elapsedTime < duration)
            {
                Fl::check();

                const auto& now = std::chrono::high_resolution_clock::now();
                elapsedTime =
                    std::chrono::duration_cast< std::chrono::milliseconds>(
                        now - start)
                        .count();
            }
        }
    }
}
