
#include <pxr/pxr.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/stage.h>

#include <vulkan/vulkan.h>

#include "USDMaterial.h"

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

        void
        CollectTextures(Fl_Vk_Context& ctx,
                        const std::unordered_map<std::string, Material >& materials,
                        std::unordered_map<std::string, std::shared_ptr<vlk::Texture > >& textureCache,
                        std::unordered_map<std::string, ShaderTextures >& collectedTextures);
    }
}
