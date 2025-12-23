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

    } // namespace vlk
} // namespace tl
