#version 450
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable

#define SLICES 12
#define PQ_BITS 14
#define PQ_MAX ((1 << PQ_BITS) - 1)
#define HIST_BITS 7
#define HIST_BIAS 64
#define HIST_BINS 64

// Nvidia RTX 3080 has a warp size of 32. 
// A 16x16 local size gives us exactly 8 warps per workgroup.
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D img;

struct PeakData {
    uint frame_wg_count[SLICES];
    uint frame_wg_active[SLICES];
    uint frame_sum_pq[SLICES];
    uint frame_max_pq[SLICES];
    uint frame_hist[SLICES * HIST_BINS];
};

layout(std430, set = 0, binding = 1) buffer PeakBuffer {
    PeakData data;
};

// Use shared memory for workgroup-wide reduction
shared uint wg_sum;
shared uint wg_max;
shared uint wg_active_count;
shared uint wg_hist[HIST_BINS];

void main() {
    uvec2 pos = gl_GlobalInvocationID.xy;
    uvec2 img_size = textureSize(img, 0);
    
    // On NVIDIA, early returns are fine as long as they don't bypass 
    // a barrier that other threads in the same warp need.
    bool within_bounds = (pos.x < img_size.x && pos.y < img_size.y);

    uint local_idx = gl_LocalInvocationIndex;
    uint wg_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
    
    // 1. Fast Parallel Clear
    if (local_idx == 0) {
        wg_sum = 0;
        wg_max = 0;
        wg_active_count = 0;
    }
    for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
        wg_hist[i] = 0;
    }
    
    // Sync shared memory
    barrier();

    uint y_pq = 0;
    bool is_active = false;

    if (within_bounds) {
        vec3 color = texture(img, vec2(pos) / vec2(img_size)).rgb;
        // RTX 3080 handles max() extremely fast
        float luma = max(color.r, max(color.g, color.b));
        
        // Fast PQ OETF approximation or full precision
        luma = clamp(luma, 0.0, 1.0);
        float l = pow(luma, 0.1593017578125);
        l = (0.8359375 + 18.8515625 * l) / (1.0 + 18.6875 * l);
        l = pow(l, 78.84375);
        
        y_pq = uint(l * PQ_MAX + 0.5);
        is_active = (y_pq > 0);
    }

    // 2. Warp-Level Reduction (RTX 3080/Ampere optimized)
    // Subgroup functions map directly to NVIDIA 'shfl' and 'popc' instructions
    uint warp_sum = subgroupAdd(y_pq);
    uint warp_max = subgroupMax(y_pq);
    uint warp_active = subgroupAdd(is_active ? 1u : 0u);

    // Only one thread per warp updates shared memory (Reduces bank conflicts)
    if (subgroupElect()) {
        atomicAdd(wg_sum, warp_sum);
        atomicMax(wg_max, warp_max);
        atomicAdd(wg_active_count, warp_active);
    }

    // 3. Histogram Update with Shared Memory
    if (within_bounds && is_active) {
        int bin = int(y_pq >> (PQ_BITS - HIST_BITS)) - HIST_BIAS;
        if (bin >= 0 && bin < HIST_BINS) {
            // Ampere has high-speed shared atomics
            atomicAdd(wg_hist[bin], 1u);
        }
    }

    barrier();

    // 4. Global SSBO Update (One thread per workgroup)
    if (local_idx == 0) {
        // Use a modulo-based slice to distribute atomic pressure across the SSBO
        uint slice = (gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES;
        
        atomicAdd(data.frame_wg_count[slice], 1u);
        
        if (wg_active_count > 0) {
            atomicAdd(data.frame_wg_active[slice], 1u);
            // On high-end GPUs, we can calculate the average here
            atomicAdd(data.frame_sum_pq[slice], wg_sum / wg_active_count);
            atomicMax(data.frame_max_pq[slice], wg_max);
        }
    }

    // 5. Global Histogram Merge
    // Distribute the 64 bins across the 256 threads for a single coalesced write
    uint slice_offset = ((gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES) * HIST_BINS;
    if (local_idx < HIST_BINS) {
        uint val = wg_hist[local_idx];
        if (val > 0) {
            atomicAdd(data.frame_hist[slice_offset + local_idx], val);
        }
    }
}
