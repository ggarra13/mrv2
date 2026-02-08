
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

            static void process_peak_data(const PeakData& data,
                                          const float percentile,
                                          const float smoothing_period,
                                          const float scene_threshold_low,
                                          const float scene_threshold_high,
                                          const bool allow_delayed,
                                          const float previous_avg_nits,
                                          float& current_avg_nits,
                                          float& current_peak_nits)
            {
                // Aggregate
                uint32_t total_wg_active = 0;
                uint64_t total_sum_pq = 0;
                uint32_t global_max_pq = 0;
                for (uint32_t s = 0; s < SLICES; ++s)
                {
                    total_wg_active += data.frame_wg_active[s];
                    total_sum_pq += static_cast<uint64_t>(data.frame_sum_pq[s]);
                    global_max_pq = std::max(global_max_pq,
                                             data.frame_max_pq[s]);
                }

                float avg_pq = 0.0f;
                float max_pq = static_cast<float>(global_max_pq) / PQ_MAX;
                if (total_wg_active > 0)
                {
                    avg_pq = static_cast<float>(total_sum_pq) / total_wg_active / PQ_MAX;
                }

                // Histogram for percentile (skip if 100.0)
                if (percentile > 0.0f && percentile < 100.0f) {
                    std::vector<uint32_t> hist(HIST_BINS, 0);
                    uint64_t total_pixels = 0;
                    for (uint32_t s = 0; s < SLICES; ++s) {
                        for (uint32_t i = 0; i < HIST_BINS; ++i) {
                            hist[i] += data.frame_hist[s * HIST_BINS + i];
                            total_pixels += data.frame_hist[s * HIST_BINS + i];
                        }
                    }

                    if (total_pixels > 0)
                    {
                        uint64_t target = static_cast<uint64_t>(percentile / 100.0f * total_pixels);
                        uint64_t sum_pixels = 0;
                        for (uint32_t i = 0; i < HIST_BINS; ++i)
                        {
                            sum_pixels += hist[i];
                            if (sum_pixels >= target)
                            {
                                float pq_low = static_cast<float>(((i) + HIST_BIAS) << (PQ_BITS - HIST_BITS)) / PQ_MAX;
                                float pq_high = (i == HIST_BINS - 1) ? max_pq : static_cast<float>(((i + 1) + HIST_BIAS) << (PQ_BITS - HIST_BITS)) / PQ_MAX;
                                float ratio = static_cast<float>(target - (sum_pixels - hist[i])) / hist[i];
                                max_pq = pq_low + ratio * (pq_high - pq_low);
                                break;
                            }
                        }
                    }
                }

                // Clamp for safety
                avg_pq = std::clamp(avg_pq, 0.0f, 1.0f);
                max_pq = std::clamp(max_pq, 0.0f, 1.0f);

                // Convert to nits
                float avg_nits = pq_eotf(avg_pq);
                float max_nits = pq_eotf(max_pq);

                // Scene change detection (using dB thresholds)
                float delta_db = 20.0f *
                                 std::log10(avg_nits /
                                            (previous_avg_nits + 1e-6f));  // Avoid divide by 0
                float abs_delta = std::fabs(delta_db);

                float coeff = (smoothing_period > 0.0f) ? 1.0f - std::exp(-1.0f / smoothing_period) : 1.0f;

                if (abs_delta > scene_threshold_high)
                {
                    // Full reset for strong change
                    coeff = 1.0f;
                }
                else if (abs_delta > scene_threshold_low)
                {
                    // Gradual reset: Scale coeff down linearly
                    float scale = (scene_threshold_high - abs_delta) / (scene_threshold_high - scene_threshold_low);
                    coeff *= scale;
                }  // Else: Normal smoothing
                
                // Apply smoothing (or reset)
                current_avg_nits += coeff * (avg_nits - current_avg_nits);
                current_peak_nits += coeff * (max_nits - current_peak_nits);
            }

            // Function to process the mapped SSBO data
            void process_peak_data(const std::shared_ptr<vlk::Shader> shader,
                                   const float percentile,
                                   const float smoothing_period,
                                   const float scene_threshold_low,
                                   const float scene_threshold_high,
                                   const bool allow_delayed,
                                   const float previous_avg_nits,
                                   float& current_avg_nits,
                                   float& current_peak_nits)
            {
                PeakData data;
                void* mapped = nullptr;
                mapped = shader->mapSSBO("PeakData");
                std::memcpy(&data, mapped, sizeof(PeakData));
                shader->unmapSSBO("PeakData");
                
                process_peak_data(data, percentile, smoothing_period,
                                  scene_threshold_low,
                                  scene_threshold_high,
                                  allow_delayed, previous_avg_nits,
                                  current_avg_nits, current_peak_nits);
            }
        }
    }
}
