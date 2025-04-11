// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

namespace tl
{
    namespace io
    {
        void normalizeImage(
            math::Vector4f& minimum, math::Vector4f& maximum,
            const std::shared_ptr<image::Image> in, const image::Info& info,
            const int minX, const int maxX, const int minY, const int maxY);

        std::string serialize(const math::Vector4f& value);
    } // namespace io
} // namespace tl
