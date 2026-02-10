
#pragma once

namespace tl
{
    namespace vlk
    {
        struct DescriptorSetLayout {
            VkDevice device;
            VkDescriptorSetLayout handle = VK_NULL_HANDLE;

            DescriptorSetLayout(VkDevice d, VkDescriptorSetLayout h) :
                device(d),
                handle(h)
                {
                }
            
            ~DescriptorSetLayout() {
                if (handle != VK_NULL_HANDLE)
                    vkDestroyDescriptorSetLayout(device, handle, nullptr);
            }
        };
    }
}
