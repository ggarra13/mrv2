
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
            struct SortKey
            {
                float depth = 0.F;        // primary: back-to-front
                uint32_t tieBreaker = 0;  // stable ordering
            };
            
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

            GfBBox3d bbox;
            SortKey sortKey;

            bool operator==(const TransparentPrimitive& b) const
                {
                    return (geom->v.size() == b.geom->v.size() &&
                            geom->t.size() == b.geom->t.size() &&
                            geom->n.size() == b.geom->n.size() &&
                            geom->c.size() == b.geom->c.size() &&
                            geom->triangles.size() == b.geom->triangles.size() &&
                            bbox == b.bbox &&
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
        
        float computeDepth(
            const GfBBox3d& bbox,
            const GfVec3d& cameraPos,
            const GfVec3d& viewDir)
        {
            GfVec3d center = bbox.ComputeCentroid();

            // Base depth
            float depth = GfDot(center - cameraPos, viewDir);

            // Add radius bias (important!)
            GfRange3d range = bbox.ComputeAlignedRange();
            float radius = 0.5f * (range.GetMax() - range.GetMin()).GetLength();

            return depth + radius;
        }
        
        TransparentPrimitive::SortKey makeSortKey(
            const GfBBox3d& bbox,
            const GfVec3d& cameraPos,
            const GfVec3d& viewDir,
            const uint32_t stableId)
        {
            TransparentPrimitive::SortKey k;

            // Project onto view direction (camera forward)
            k.depth = computeDepth(bbox, cameraPos, viewDir);

            // Stable fallback (object ID, pointer hash, etc.)
            k.tieBreaker = stableId;

            return k;
        }
    }
}
