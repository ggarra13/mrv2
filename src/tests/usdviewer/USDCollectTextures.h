
#include <pxr/pxr.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/stage.h>

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <string>

class Fl_Vk_Context;

namespace tl
{
    namespace vlk
    {
        class Texture;
    }
    
    namespace usd
    {
        using namespace PXR_NS;

        // Slots to use for Vulkan textures in a shader.
        typedef std::unordered_map<int,
                                   std::shared_ptr<vlk::Texture> > ShaderTextures;

        // Collect all textures for a primitive.
        void CollectTextures(Fl_Vk_Context& ctx,
                             const VkCommandBuffer cmd,
                             const UsdStageRefPtr stage,
                             const UsdTimeCode time,
                             std::unordered_map<std::string, ShaderTextures >&
                             collectedTextures);
    }
}
