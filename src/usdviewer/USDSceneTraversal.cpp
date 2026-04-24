
#include <pxr/usd/usd/primRange.h>

#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>

#include "USDRenderEngine.h"  // must come last

namespace tl
{
    namespace usd
    {
        void RenderEngine::_sceneTraversal()
        {
            TLRENDER_P();
            
            using namespace pxr;
        
            TfTokenVector purposes = {UsdGeomTokens->default_};
            UsdGeomBBoxCache bboxCache(p.time, purposes);
    
            UsdPrimRange range(p.stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());
            GfMatrix4d matrix;
            UsdGeomXformCache xformCache(p.time);
            std::string primPath;

            //
            // Stats
            //
            p.stats.reset();
            p.opaquePrims.clear();
            p.transparentPrims.clear();
            
            std::shared_ptr<vlk::Texture> texture;    
            for (auto it = range.begin(); it != range.end(); ++it) {

                // Check if a primitive is visible and its purpose is "render" or "default".
                if (it->IsA<UsdGeomImageable>()) {
                    UsdGeomImageable imageable(*it);
            
                    if (imageable.ComputeVisibility(p.time) ==
                        UsdGeomTokens->invisible) {
                        // If this prim is invisible, its entire subtree is invisible.
                        // Prune the traversal to skip all children.
                        it.PruneChildren();
                        continue;
                    }

                    // If purpose is not default or not render, don't use this
                    // geometry.
                    TfToken purpose = imageable.ComputePurpose();
                    if (purpose != UsdGeomTokens->default_ &&
                        purpose != UsdGeomTokens->render)
                        continue;

                }

                if (!UsdShadeMaterialBindingAPI::CanApply(*it))
                    continue;

                primPath = it->GetPath().GetString();


                matrix = xformCache.GetLocalToWorldTransform(*it);
                const math::Matrix4x4f modelMatrix(matrix[0][0], matrix[0][1],
                                                   matrix[0][2], matrix[0][3],
                                                   matrix[1][0], matrix[1][1],
                                                   matrix[1][2], matrix[1][3],
                                                   matrix[2][0], matrix[2][1],
                                                   matrix[2][2], matrix[2][3],
                                                   matrix[3][0], matrix[3][1],
                                                   matrix[3][2], matrix[3][3]);

                // std::regex re("Leg");
                //std::regex re("REye");
                //std::regex re("(?:Iris|Pupil|Sclera)");  // renders brown as it should
                // std::regex re("Sclera");
                // std::regex re("Cornea");
                // std::regex re("Pupil");  // renders black as it should
                
                // if (!std::regex_search(primPath, re))
                //     continue;
                
                // std::cout << primPath << std::endl;

                image::Color4f color(1, 1, 1);
                
                VtArray<GfVec3f> colors;
                UsdGeomGprim gprim(*it);
                if (gprim)
                    gprim.GetDisplayColorAttr().Get(&colors, p.time);

                if (colors.size() == 1)
                {
                    color.r = colors[0][0];
                    color.g = colors[0][1];
                    color.b = colors[0][2];
                    //color.a = colors[0][3];  // alpha is not used.
                }
                
                
                std::string shaderId;
                if (it->IsA<UsdGeomMesh>())
                {
                    p.stats.total++;
                    p.stats.meshes++;

                    UsdGeomMesh usdMesh = UsdGeomMesh(*it);
                    _drawMesh(primPath, usdMesh, modelMatrix, shaderId, color,
                              bboxCache);
                }
                else if (it->IsA<UsdGeomNurbsPatch>())
                {
                    p.stats.total++;
                    p.stats.nurbs++; 
                    UsdGeomNurbsPatch out = UsdGeomNurbsPatch(*it);
                }
                else if (it->IsA<UsdGeomNurbsCurves>())
                {
                    p.stats.total++;
                    p.stats.nurbsCurves++; 
                    UsdGeomNurbsCurves out = UsdGeomNurbsCurves(*it);
                }
                else if (it->IsA<UsdGeomBasisCurves>())
                {
                    p.stats.total++;
                    p.stats.basisCurves++; 
                    UsdGeomBasisCurves out = UsdGeomBasisCurves(*it);
                }
                else if (it->IsA<UsdGeomSphere>())
                {
                    p.stats.total++;
                    p.stats.spheres++;

                    UsdTimeCode time = p.time;
                    
                    UsdGeomSphere out = UsdGeomSphere(*it);
                    float radius = 1;
                    out.GetRadiusAttr().Get(&radius, time);
                    auto geom = geom::sphere(radius, 16, 16);
                    
                    std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                    UsdShadeMaterialBindingAPI api(*it);
                    UsdShadeMaterial material = usd::GetMaterial(api);

                    const std::string materialKey = material.GetPath().GetString();
                    auto i = p.textures.find(materialKey);
                    if (i != p.textures.end())
                    {
                        textures = i->second;
                        shaderId = "UsdShaderPreview";
                    }

                    MeshOptimization opt;
                    p.render->drawMesh(geom, opt, modelMatrix, color,
                                       shaderId, textures);
                }
                else if (it->IsA<UsdGeomCube>())
                {
                    p.stats.total++;
                    p.stats.cubes++;
                }
                else if (it->IsA<UsdGeomCylinder>())
                {
                    p.stats.total++;
                    p.stats.cylinders++;
                }
                else if (it->IsA<UsdGeomCapsule>())
                {
                    p.stats.total++;
                    p.stats.capsules++;
                }
                else if (it->IsA<UsdGeomCone>())
                {
                    p.stats.total++;
                    p.stats.cones++;
                }
            }
        }
    }
}
