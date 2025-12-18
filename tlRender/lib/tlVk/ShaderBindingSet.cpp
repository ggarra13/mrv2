// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlVk/ShaderBindingSet.h>

namespace tl
{
    namespace vlk
    {

        ShaderBindingSet::ShaderBindingSet(VkDevice device) :
            device(device)
        {
            descriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
            descriptorPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
        }
        
        ShaderBindingSet::~ShaderBindingSet()
        {
            destroy();
        }
        
        void ShaderBindingSet::destroy()
        {
            for (auto& [_, ubo] : uniforms)
            {
                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                {
                    if (ubo.buffers[i] != VK_NULL_HANDLE)
                    {
                        vkDestroyBuffer(device,
                                        ubo.buffers[i],
                                        nullptr);
                        ubo.buffers[i] = VK_NULL_HANDLE;
                    }
                    if (ubo.memories[i] != VK_NULL_HANDLE)
                    {
                        vkFreeMemory(device,
                                     ubo.memories[i],
                                     nullptr);
                        ubo.memories[i] = VK_NULL_HANDLE;
                    }
                }
            }

            for (auto& [_, sb] : storageBuffers)
            {
                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                {
                    sb.buffers[i].reset();
                }
            }
            
            for (auto& pool : descriptorPools)
            {
                if (pool != VK_NULL_HANDLE)
                {
                    vkDestroyDescriptorPool(device, pool, nullptr);
                    pool = VK_NULL_HANDLE;
                }
            }

            descriptorSets.clear();
            descriptorPools.clear();

            storageBuffers.clear();
            storageImages.clear();
            uniforms.clear();
            textures.clear();
            fbos.clear();
        }            

    } // namespace vlk
} // namespace tl
