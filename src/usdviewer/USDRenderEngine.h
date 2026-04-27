
#include <tlVk/OffscreenBuffer.h>

#include <tlCore/Matrix.h>
#include <tlCore/Util.h>

#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/subset.h>

#include <memory>

namespace tl
{
    namespace usd
    {
        using namespace pxr;
        
        class RenderEngine : public std::enable_shared_from_this<RenderEngine>
        {
            TLRENDER_NON_COPYABLE(RenderEngine);

            void _init();
    
        public:
            RenderEngine(Fl_Vk_Context& ctx);
            ~RenderEngine();
    
            //! Create a new render engine.
            static std::shared_ptr<RenderEngine> create(Fl_Vk_Context& ctx);

            std::shared_ptr<vlk::OffscreenBuffer> getFBO();
    
            void setTimeCode(UsdStageRefPtr stage, const UsdTimeCode& time);

            void draw(VkCommandBuffer, unsigned frameIndex,
                      unsigned renderWidth);
            
        protected:
            void _sceneTraversal();
            void _bakeJoints();
            
            std::vector<UsdGeomMesh> _SplitMeshBySubsets(
                const std::string& primPath,
                const UsdGeomMesh& mesh,
                const VtArray<GfVec3f>& points,
                const VtArray<int>& faceCounts,
                const VtArray<int> faceIndices,
                const std::vector<UsdGeomSubset>& subsets);
            void _drawMesh(const std::string& primPath,
                           const UsdGeomMesh& usdMesh,
                           const math::Matrix4x4f& modelMatrix,
                           std::string shaderName,
                           const image::Color4f& color,
                           pxr::UsdGeomBBoxCache& );

            Fl_Vk_Context& ctx;
            
            TLRENDER_PRIVATE();
        };
    }
}

#include "USDRenderEnginePrivate.h"
