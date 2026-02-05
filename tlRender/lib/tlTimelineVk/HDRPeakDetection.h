#pragma once

#include <tlVk/Shader.h>

namespace tl
{
    namespace timeline_vlk
    {
        namespace hdr
        {
            // Constants matching the shader
            constexpr uint32_t SLICES = 12;
            constexpr uint32_t PQ_BITS = 14;
            constexpr uint32_t PQ_MAX = (1 << PQ_BITS) - 1;
            constexpr uint32_t HIST_BITS = 7;
            constexpr uint32_t HIST_BIAS = (1 << (HIST_BITS - 1));
            constexpr uint32_t HIST_BINS = ((1 << HIST_BITS) - HIST_BIAS);
        
            constexpr float PQ_M1 = 0.1593017578125f;
            constexpr float PQ_M2 = 78.84375f;
            constexpr float PQ_C1 = 0.8359375f;
            constexpr float PQ_C2 = 18.8515625f;
            constexpr float PQ_C3 = 18.6875f;
            
            struct PeakData {
                uint32_t frame_wg_count[SLICES];
                uint32_t frame_wg_active[SLICES];
                uint32_t frame_sum_pq[SLICES];
                uint32_t frame_max_pq[SLICES];
                uint32_t frame_hist[SLICES * HIST_BINS];
            };// PQ EOTF function (linear light from PQ, scaled)
            
            //! Function to process the mapped SSBO data
            void process_peak_data(
                const std::shared_ptr<vlk::Shader> shader,
                float percentile, float smoothing_period,
                float& current_avg_nits,
                float& current_peak_nits);

        }


    }
}
