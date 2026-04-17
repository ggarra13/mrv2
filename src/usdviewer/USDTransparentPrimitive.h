
#include "USDMaterial.h"

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
            geom::TriangleMesh3 geom;
            MeshOptimization optimization;

            math::Matrix4x4f modelMatrix;
            
            // Material information.
            std::string shaderName;
            image::Color4f color;
            Material material;
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            
            math::Vector3f center;

            bool operator==(const TransparentPrimitive& b) const
                {
                    return (geom.v.size() == b.geom.v.size() &&
                            geom.t.size() == b.geom.t.size() &&
                            geom.n.size() == b.geom.n.size() &&
                            geom.triangles.size() == b.geom.triangles.size() &&
                            modelMatrix == b.modelMatrix &&
                            shaderName == b.shaderName &&
                            color == b.color &&
                            material == b.material &&
                            textures == b.textures &&
                            center == b.center);
                }
            
            bool operator!=(const TransparentPrimitive& b) const
                {
                    return !(*this == b);
                }
        };
    }
}
