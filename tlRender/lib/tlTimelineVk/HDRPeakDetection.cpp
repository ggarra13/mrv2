
#include "tlTimelineVk/HDRPeakDetection.h"

#include <cmath>

namespace tl
{
    namespace timeline_vlk
    {
        namespace hdr
        {
            inline float pq_eotf(float pq) {
                float y = std::pow(pq, 1.0f / PQ_M2);
                y = std::max(y - PQ_C1, 0.0f) / (PQ_C2 - PQ_C3 * y);
                y = std::pow(y, 1.0f / PQ_M1);
                return y * 10000.0f;  // To absolute nits
            }
            
            // Function to process the mapped SSBO data
            void process_peak_data(const std::shared_ptr<vlk::Shader> shader,
                                   float percentile, float smoothing_period,
                                   float& current_avg_nits,
                                   float& current_peak_nits)
            {
                PeakData data;
                void* mapped = nullptr;
                mapped = shader->mapSSBO("PeakData");
                std::memcpy(&data, mapped, sizeof(PeakData));
                shader->unmapSSBO("PeakData");
                
                // Aggregate
                uint32_t total_wg_active = 0;
                uint64_t total_sum_pq = 0;  // Use 64-bit to avoid overflow
                uint32_t global_max_pq = 0;
                for (uint32_t s = 0; s < SLICES; ++s) {
                    total_wg_active += data.frame_wg_active[s];
                    total_sum_pq += static_cast<uint64_t>(data.frame_sum_pq[s]);
                    global_max_pq = std::max(global_max_pq, data.frame_max_pq[s]);
                }

                float avg_pq = 0.0f;
                float max_pq = static_cast<float>(global_max_pq) / PQ_MAX;
                max_pq = std::clamp(max_pq, 0.0f, 1.0f);
                if (total_wg_active > 0) {
                    avg_pq = static_cast<float>(total_sum_pq) / total_wg_active / PQ_MAX;
                }
                avg_pq = std::clamp(avg_pq, 0.0f, 1.0f);

                // Histogram for percentile
                std::vector<uint32_t> hist(HIST_BINS, 0);
                uint64_t total_pixels = 0;
                for (uint32_t s = 0; s < SLICES; ++s) {
                    for (uint32_t i = 0; i < HIST_BINS; ++i) {
                        hist[i] += data.frame_hist[s * HIST_BINS + i];
                        total_pixels += data.frame_hist[s * HIST_BINS + i];
                    }
                }

                if (percentile > 0.0f && percentile < 100.0f &&
                    total_pixels > 0)
                {
                    uint64_t target = static_cast<uint64_t>(percentile / 100.0f * total_pixels);
                    uint64_t sum_pixels = 0;
                    for (uint32_t i = 0; i < HIST_BINS; ++i) {
                        sum_pixels += hist[i];
                        if (sum_pixels >= target) {
                            float pq_low = static_cast<float>(((i) + HIST_BIAS) << (PQ_BITS - HIST_BITS)) / PQ_MAX;
                            float pq_high = (i == HIST_BINS - 1) ? max_pq : static_cast<float>(((i + 1) + HIST_BIAS) << (PQ_BITS - HIST_BITS)) / PQ_MAX;
                            float ratio = static_cast<float>(target - (sum_pixels - hist[i])) / hist[i];
                            max_pq = pq_low + ratio * (pq_high - pq_low);
                            break;
                        }
                    }
                }

                // Convert to nits (if needed; here avg and peak in nits)
                float avg_nits = pq_eotf(avg_pq);
                float max_nits = pq_eotf(max_pq);

                // Smoothing (IIR filter)
                float coeff = (smoothing_period > 0.0f) ? 1.0f - std::exp(-1.0f / smoothing_period) : 1.0f;
                current_avg_nits += coeff * (avg_nits - current_avg_nits);
                current_peak_nits += coeff * (max_nits - current_peak_nits);

                // Scene change handling (optional; implement based on avg/peak delta or external logic)
                // ...
            }
        }
    }
}
