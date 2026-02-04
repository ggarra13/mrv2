#version 450
#extension GL_EXT_shader_atomic_float : enable  // For atomicAdd on floats if needed, but we use uint
#extension GL_KHR_shader_subgroup_basic : enable  // For subgroups, optional
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable

// Constants from libplacebo
#define SLICES 12
#define PQ_BITS 14
#define PQ_MAX ((1 << PQ_BITS) - 1)
#define HIST_BITS 7
#define HIST_BIAS (1 << (HIST_BITS - 1))  // 64
#define HIST_BINS ((1 << HIST_BITS) - HIST_BIAS)  // 64
#define HIST_PQ(bin) (((bin) + HIST_BIAS) << (PQ_BITS - HIST_BITS))

#define PQ_M1 0.1593017578125
#define PQ_M2 78.84375
#define PQ_C1 0.8359375
#define PQ_C2 18.8515625
#define PQ_C3 18.6875

#define PL_COLOR_SDR_WHITE 203.0

// Input: PQ-encoded HDR frame (BT.2020 RGB)
layout(set = 0, binding = 0) uniform sampler2D img;

// Output SSBO
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

// Luma coefficients for BT.2020
const vec3 LUMA_COEFFS = vec3(0.2627, 0.6780, 0.0593);

// Shared memory for workgroup reduction
shared uint wg_sum;
shared uint wg_max;
shared uint wg_black;
shared uint wg_hist[HIST_BINS];

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

void main() {
    uvec2 pos = gl_GlobalInvocationID.xy;
    uvec2 img_size = textureSize(img, 0);
    if (pos.x >= img_size.x || pos.y >= img_size.y) return;

    // Sample color (PQ-encoded)
    vec3 color = texture(img, vec2(pos) / vec2(img_size)).rgb;

    // // Apply PQ EOTF to linear light (normalized so 1.0 = 10000 nits initially)
    // vec3 Y = pow(color, vec3(1.0 / PQ_M2));
    // color = max(Y - PQ_C1, 0.0) / (PQ_C2 - PQ_C3 * Y);
    // color = pow(color, vec3(1.0 / PQ_M1));

    // // Scale to make 1.0 = PL_COLOR_SDR_WHITE nits
    // color *= (10000.0 / PL_COLOR_SDR_WHITE);

    // Compute luma in units where 1.0 = PL_COLOR_SDR_WHITE nits
    float luma = dot(color, LUMA_COEFFS);

    // Scale for PQ OETF input (to nits / 10000)
    luma *= (PL_COLOR_SDR_WHITE / 10000.0);

    // Clamp and apply PQ OETF
    luma = clamp(luma, 0.0, 1.0);
    luma = pow(luma, PQ_M1);
    luma = (PQ_C1 + PQ_C2 * luma) / (1.0 + PQ_C3 * luma);
    luma = pow(luma, PQ_M2);
    uint y_pq = uint(luma * PQ_MAX + 0.5);  // Round to nearest

    // Workgroup slice
    uint wg_idx = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
    uint slice = wg_idx % SLICES;
    uint hist_base = slice * HIST_BINS;

    // Initialize shared memory
    uint local_idx = gl_LocalInvocationIndex;
    uint wg_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    if (local_idx == 0) {
        wg_sum = 0;
        wg_max = 0;
        wg_black = 0;
    }
    for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
        wg_hist[i] = 0;
    }
    barrier();

    // Update local histogram
    int bin = int(y_pq >> (PQ_BITS - HIST_BITS));
    bin -= HIST_BIAS;
    bin = clamp(bin, 0, HIST_BINS - 1);
    atomicAdd(wg_hist[bin], 1u);

    // Update sum/max/black using subgroups if available
    if (subgroupElect()) {  // First lane in subgroup
        uint group_sum = subgroupAdd(y_pq);
        uint group_max = subgroupMax(y_pq);
        uvec4 ballot = subgroupBallot(y_pq == 0u);
        uint black_count = bitCount(ballot.x) + bitCount(ballot.y) + bitCount(ballot.z) + bitCount(ballot.w);  // For up to 128-wide, but typically 32/64
        atomicAdd(wg_sum, group_sum);
        atomicMax(wg_max, group_max);
        atomicAdd(wg_black, black_count);
    }
    barrier();

    // Adjust hist[0] for black pixels (y_pq == 0)
    if (local_idx == 0) {
        wg_hist[0] -= wg_black;
    }
    barrier();

    // Update global histogram using atomics
    for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
        if (wg_hist[i] > 0) {
            atomicAdd(data.frame_hist[hist_base + i], wg_hist[i]);
        }
    }

    // One thread updates global stats
    if (local_idx == 0) {
        uint num_pixels = wg_size - wg_black;
        atomicAdd(data.frame_wg_count[slice], 1u);
        if (num_pixels > 0) {
            atomicAdd(data.frame_wg_active[slice], 1u);
            atomicAdd(data.frame_sum_pq[slice], wg_sum / num_pixels);  // Average per active wg
            atomicMax(data.frame_max_pq[slice], wg_max);
        }
    }
}
