// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlVk/Vk.h>

#include <functional>
#include <stdexcept>
#include <unordered_map>

// Hash function for VkSamplerCreateInfo (simplified)
struct SamplerDescHash {
    std::size_t operator()(const VkSamplerCreateInfo& info) const {
        // Just hash a subset of the fields relevant for uniqueness
        return std::hash<uint32_t>()(info.magFilter) ^
               std::hash<uint32_t>()(info.minFilter) ^
               std::hash<uint32_t>()(info.addressModeU) ^
               std::hash<uint32_t>()(info.addressModeV) ^
               std::hash<uint32_t>()(info.addressModeW);
    }
};

// Equality function for unordered_map
struct SamplerDescEqual {
    bool operator()(const VkSamplerCreateInfo& a,
                    const VkSamplerCreateInfo& b) const {
        return a.magFilter == b.magFilter &&
               a.minFilter == b.minFilter &&
               a.addressModeU == b.addressModeU &&
               a.addressModeV == b.addressModeV &&
               a.addressModeW == b.addressModeW;
    }
};

class SamplersCache {
public:
    SamplersCache(VkDevice device) : device(device) {}

    ~SamplersCache() {
        for (auto& entry : cache) {
            vkDestroySampler(device, entry.second, nullptr);
        }
    }

    VkSampler getOrCreateSampler(const VkSamplerCreateInfo& info) {
        auto it = cache.find(info);
        if (it != cache.end())
            return it->second;

        VkSampler sampler;
        VkResult result = vkCreateSampler(device, &info, nullptr, &sampler);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create sampler");

        cache[info] = sampler;
        return sampler;
    }

private:
    VkDevice device;
    std::unordered_map<VkSamplerCreateInfo, VkSampler, SamplerDescHash, SamplerDescEqual> cache;
};
