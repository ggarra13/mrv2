// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

namespace tl
{
    namespace usd
    {
        struct USDTransforms
        {
            alignas(16) math::Matrix4x4f mvp;
            alignas(16) math::Matrix4x4f model;
        };
        
        struct PBRTransform
        {
            alignas(16) math::Matrix4x4f mvp;
            alignas(16) math::Matrix4x4f model;
            alignas(16) math::Matrix3x3f normalMatrix;
        };
                
        struct PBRMaterial
        {
            alignas(16) math::Vector4f diffuseColor;
            alignas(4)  float metallic;
            alignas(4)  float roughtness;
            alignas(4)  float aoStrength;
        };
        
        struct PBRScene
        {
            alignas(16) math::Vector3f camPos;
            alignas(16) math::Vector3f lightPos;
            alignas(16) math::Vector3f lightColor;
        };
        
    }
}
