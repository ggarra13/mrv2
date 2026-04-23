
#include "USDMaterial.h"
#include "USDMeshOptimization.h"

#include <tlCore/Color.h>
#include <tlCore/Matrix.h>
#include <tlCore/Mesh.h>

namespace tl
{
    namespace usd
    {
        
        struct TransparentPrimitive
        {
                                                  
            // Geometry information.
            std::shared_ptr<geom::TriangleMesh3> geom;
            MeshOptimization optimization;

            math::Matrix4x4f modelMatrix;
            
            // Material information.
            std::string shaderId;
            image::Color4f color;
            Material material;
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;

            bool operator==(const TransparentPrimitive& b) const
                {
                    return (geom->v.size() == b.geom->v.size() &&
                            geom->t.size() == b.geom->t.size() &&
                            geom->n.size() == b.geom->n.size() &&
                            geom->c.size() == b.geom->c.size() &&
                            geom->triangles.size() == b.geom->triangles.size() &&
                            modelMatrix == b.modelMatrix &&
                            shaderId == b.shaderId &&
                            color == b.color &&
                            material == b.material &&
                            textures == b.textures);
                }
            
            bool operator!=(const TransparentPrimitive& b) const
                {
                    return !(*this == b);
                }
        };
        
    }
}
