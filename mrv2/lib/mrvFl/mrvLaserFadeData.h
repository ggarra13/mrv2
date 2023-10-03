// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvDraw/Annotation.h"
#include "mrvDraw/Shape.h"

namespace mrv
{
    using namespace tl;

    class TimelineViewport;

    struct LaserFadeData
    {
        TimelineViewport* view;
        std::shared_ptr<draw::Annotation> annotation;
        std::shared_ptr<draw::Shape> shape;
    };
} // namespace mrv
