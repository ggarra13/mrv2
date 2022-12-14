// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/BBox.h>
#include <tlCore/Color.h>

namespace tl
{
    namespace gl
    {
        //! OpenGL renderer.
        class Outline
        {
        public:
            Outline();
            ~Outline();

            void drawRect(
                const math::BBox2i&,
                const imaging::Color4f&,
                const math::Matrix4x4f& mvp);

        private:
            TLRENDER_PRIVATE();
        };
        
    }
}
