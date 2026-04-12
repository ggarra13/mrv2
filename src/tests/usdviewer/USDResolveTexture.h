
#include "USDGetTextureOrValue.h"

#include <memory>
#include <string>

class Fl_Vk_Context;

namespace tl
{
    namespace vlk
    {
        class Texture;
        
        std::shared_ptr<Texture> ResolveTexture(
            Fl_Vk_Context& ctx,
            const usd::ShaderInputResult& result);
    }
}
