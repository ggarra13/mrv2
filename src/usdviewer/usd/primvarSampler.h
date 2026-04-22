#pragma once

namespace tl
{
    namespace usd
    {
        template <typename T>
        class PrimvarSampler
        {
        public:
            PrimvarSampler(const UsdGeomPrimvar& primvar,
                           const UsdTimeCode& time)
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

                    if (interpolation == UsdGeomTokens->faceVarying)
                    {
                        domainIdx = faceVertexIdx;
                    }
                    else if (interpolation == UsdGeomTokens->vertex ||
                             interpolation == UsdGeomTokens->varying)
                    {
                        domainIdx = pointIdx;
                    }
                    else if (interpolation == UsdGeomTokens->uniform)
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
            TfToken interpolation;

            VtArray<T> values;
            VtArray<int> indices;
        };

    }
}
