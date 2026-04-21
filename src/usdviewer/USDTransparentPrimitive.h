
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
            TransparentPrimitive(std::shared_ptr<geom::TriangleMesh3> g,
                                 MeshOptimization& o,
                                 const math::Matrix4x4f& m,
                                 std::string& s,
                                 const image::Color4f& c,
                                 Material& mat,
                                 std::unordered_map<int, std::shared_ptr<vlk::Texture > >& txt) :
                geom(g),
                optimization(o),
                modelMatrix(m),
                shaderId(s),
                color(c),
                material(mat),
                textures(txt)
                {
                }
                               
                               
            
            // Geometry information.
            std::shared_ptr<geom::TriangleMesh3> geom;
            MeshOptimization optimization;

            math::Matrix4x4f modelMatrix;
            
            // Material information.
            std::string shaderId;
            image::Color4f color;
            Material material;
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;

            math::Vector3f center;

            bool operator==(const TransparentPrimitive& b) const
                {
                    return (center == b.center &&
                            geom->v.size() == b.geom->v.size() &&
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
