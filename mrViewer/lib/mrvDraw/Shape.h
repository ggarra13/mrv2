// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <float.h>
#include <limits.h>
#include <cmath>
#include <vector>
#include <iostream>

#include "mrvDraw/Point.h"

#include <tlCore/Vector.h>
#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

namespace tl
{

    namespace draw
    {

        class Shape
        {
        public:
            Shape() : color( 0.F, 1.F, 0.F, 1.F ),
                      pen_size(5)
                {
                };

            virtual ~Shape() {};

            virtual void draw(
                const std::shared_ptr<timeline::IRender>&) = 0;

        public:
            math::Matrix4x4f matrix;
            imaging::Color4f color;
            float pen_size;
        };

        class PathShape : public Shape
        {
        public:

            PathShape() : Shape()  {};
            virtual ~PathShape() {};
    
            virtual void draw(
                const std::shared_ptr<timeline::IRender>&) = 0;

            PointList pts;
        };

    }
}
