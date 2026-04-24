// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>

namespace tl
{
    namespace usd
    {
        struct USDTransforms
        {
            alignas(16) math::Matrix4x4f mvp;
            alignas(16) math::Matrix4x4f model;
            alignas(16) math::Matrix4x4f view;
        };
        
    }
}
