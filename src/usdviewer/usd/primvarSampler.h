#pragma once

namespace tl
{
    namespace usd
    {
        template <typename T>
        class PrimvarSampler
        {
        public:
            PrimvarSampler(const pxr::UsdGeomPrimvar& primvar,
                           const pxr::UsdTimeCode& time)
                {
                    valid = primvar.IsDefined();
                    if (!valid)
                        return;

                    primvar.Get(&values, time);
                    interpolation = primvar.GetInterpolation();
                    
                    indexed = primvar.IsIndexed();
                    if (indexed)
                        primvar.GetIndices(&indices, time);
                }

            bool IsValid() const { return valid && !values.empty(); }

            T Sample(int faceIdx,
                     int faceVertexIdx,
                     int pointIdx) const
                {
                    int idx = ResolveIndex(faceIdx, faceVertexIdx, pointIdx);
                    return values[idx];
                }

        private:
            int ResolveIndex(int faceIdx,
                             int faceVertexIdx,
                             int pointIdx) const
                {
                    int domainIdx = 0;

                    if (interpolation == pxr::UsdGeomTokens->faceVarying)
                    {
                        domainIdx = faceVertexIdx;
                    }
                    else if (interpolation == pxr::UsdGeomTokens->vertex ||
                             interpolation == pxr::UsdGeomTokens->varying)
                    {
                        domainIdx = pointIdx;
                    }
                    else if (interpolation == pxr::UsdGeomTokens->uniform)
                    {
                        domainIdx = faceIdx;
                    }
                    else // constant
                    {
                        domainIdx = 0;
                    }

                    if (indexed)
                    {
                        return indices[domainIdx];
                    }
                    
                    return domainIdx;
                }

        private:
            bool valid = false;
            bool indexed = false;
            pxr::TfToken interpolation;

            pxr::VtArray<T> values;
            pxr::VtArray<int> indices;
        };

    }
}
