// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include <tlCore/Color.h>
#include <tlCore/Image.h>

namespace mrv
{
    namespace color
    {
        using namespace tl;
        image::Color4f fromVoidPtr(const void* ptr, const image::PixelType pixelType);
    }
}
