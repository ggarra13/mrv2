// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Box.h>
#include <tlCore/Color.h>

namespace mrv
{
    namespace opengl
    {
        using namespace tl;

        //! OpenGL Outline renderer.
        class Outline
        {
        public:
            Outline();
            ~Outline();

            void drawRect(
                const math::Box2i&, const image::Color4f&,
                const math::Matrix4x4f& mvp);

        private:
            TLRENDER_PRIVATE();
        };

    } // namespace opengl
} // namespace mrv
