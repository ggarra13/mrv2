// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <limits>
#include <cmath>
#include <vector>
#include <iostream>

#include <tlCore/Vector.h>
#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

#include "mrvDraw/Point.h"

#include "mrvCore/mrvBackend.h"

#ifdef TLRENDER_VK
#  include "mrvVk/mrvVkLines.h"
#endif

#ifdef TLRENDER_GL
#  include "mrvGL/mrvGLLines.h"
#endif

namespace mrv
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

#ifdef TLRENDER_GL
            virtual void draw(
                const std::shared_ptr<tl::timeline::IRender>&,
                const std::shared_ptr<opengl::Lines>&) {};
#endif
            
#ifdef TLRENDER_VK
            virtual void draw(
                const std::shared_ptr<tl::timeline::IRender>&,
                const std::shared_ptr<vulkan::Lines>&) {};
#endif

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

#ifdef TLRENDER_GL
            virtual void draw(
                const std::shared_ptr<tl::timeline::IRender>&,
                const std::shared_ptr<opengl::Lines>&) {};
#endif
            
#ifdef TLRENDER_VK
            virtual void draw(
                const std::shared_ptr<tl::timeline::IRender>&,
                const std::shared_ptr<vulkan::Lines>&) {};
#endif

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
} // namespace mrv
