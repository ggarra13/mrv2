// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <tlDraw/Point.h>

#include <tlCore/Color.h>
#include <tlCore/Vector.h>
#include <tlCore/Matrix.h>

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>

namespace tl
{

    namespace draw
    {

        using namespace tl;

        class Shape
        {
        public:
            Shape() :
                color(0.F, 1.F, 0.F, 1.F),
                soft(false),
                laser(false),
                fade(1.0F),
                pen_size(5) {};

            virtual ~Shape() {};

            virtual void draw(const std::shared_ptr<timeline::IRender>&) {};

        public:
            math::Matrix4x4f matrix;
            image::Color4f color;
            bool soft;
            bool laser;
            float fade;
            float pen_size;
        };

        class PathShape : public Shape
        {
        public:
            PathShape() :
                Shape() {};
            virtual ~PathShape() {};

            virtual void draw(const std::shared_ptr<timeline::IRender>&) {};

            PointList pts;
        };

        class NoteShape : public Shape
        {
        public:
            NoteShape() :
                Shape() {};
            virtual ~NoteShape() {};


        public:
            std::string text;
        };

        void to_json(nlohmann::json&, const Shape&);

        void from_json(const nlohmann::json&, Shape&);

        void to_json(nlohmann::json&, const PathShape&);

        void from_json(const nlohmann::json&, PathShape&);

        void to_json(nlohmann::json&, const NoteShape&);

        void from_json(const nlohmann::json&, NoteShape&);

    } // namespace draw

} // namespace tl
