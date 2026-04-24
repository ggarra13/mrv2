
#include "USDRender/Render.h"
#include "USDRender/ShadersBinary.h"
#include "USDTransparentPrimitive.h"

#include "usd/material.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>

namespace tl
{
    namespace usd
    {
        struct RenderEngine::Private
        {    
            // USD information
            UsdStageRefPtr stage = nullptr;

            // Timeline
            double startTimeCode = 0.F;
            double endTimeCode = 100.F;
            double timeCodesPerSecond = 24.0F;
            UsdTimeCode time = 0;

            // Vulkan information.
            bool collectMaterials = true;
            bool collectTextures = true;
            std::unordered_map<std::string, usd::Material > materials;
            std::unordered_map<std::string, usd::ShaderTextures > textures;

            // Renderer information
            std::vector<TransparentPrimitive> transparentPrims;

            struct Stats
            {
                std::chrono::steady_clock::time_point timer;
                
                // Primitive counts
                std::size_t total  = 0;
                std::size_t opaque = 0;
                std::size_t transparent = 0;

                // Total scene triangles
                std::size_t triangles = 0;
                
                // Main Primitive types
                std::size_t meshes = 0;
                std::size_t subdivs = 0;
                std::size_t nurbs = 0;
                std::size_t nurbsCurves = 0;
                std::size_t basisCurves = 0;
                std::size_t points = 0;

                // Not common primitives
                std::size_t capsules = 0;
                std::size_t cones = 0;
                std::size_t cubes = 0;
                std::size_t cylinders = 0;
                std::size_t spheres = 0;
                
                std::size_t textures = 0;
                
                std::size_t skeletons = 0;

                void reset()
                    {
                        opaque = transparent = total = 0;
                        triangles = meshes = subdivs = nurbs = 0;
                        nurbsCurves = basisCurves = 0;
                        points = 0;
                        capsules = cones = cubes = cylinders = spheres = 0;
                        skeletons = 0;
                    }

                void print(std::ostream& o)
                    {
                        o << "---------------------------------------------------"
                          << std::endl
                          << "    Triangles = " << triangles
                          << std::endl
                          << "    Textures  = " << textures
                          << std::endl
                          << std::endl
                          << "        Total = " << total << std::endl
                          << "       Opaque = " << opaque << std::endl
                          << "  Transparent = " << transparent
                          << std::endl
                          << std::endl
                          << "       Meshes = " << meshes << std::endl
                          << "      Subdivs = " << subdivs << std::endl
                          << "Nurbs Patches = " << nurbs << std::endl
                          << " Nurbs Curves = " << nurbsCurves
                          << std::endl
                          << " Basis Curves = " << basisCurves
                          << std::endl
                          << "       Points = " << points << std::endl
                          << "    Skeletons = " << skeletons
                          << std::endl
                          << "     Capsules = " << capsules << std::endl
                          << "        Cones = " << cones << std::endl
                          << "        Cubes = " << cubes << std::endl
                          << "    Cylinders = " << cylinders << std::endl
                          << "      Spheres = " << spheres << std::endl
                          << std::endl;
                    }
            };
            Stats stats;
            
            //! tlRender context
            std::shared_ptr<system::Context> context;


            //! Offscreen renderer.
            std::shared_ptr<usd::Render> render;
    
            //! Offscreen buffer.
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
        };

    }
}
