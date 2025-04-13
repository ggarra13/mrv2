// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace tk
    {
        //! Get the glReadPixels format.
        unsigned int getReadPixelsFormat(image::PixelType);

        //! Get the glReadPixels type.
        unsigned int getReadPixelsType(image::PixelType);
    } // namespace tk
} // namespace tl
