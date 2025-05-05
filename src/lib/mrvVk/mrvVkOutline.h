// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Box.h>
#include <tlCore/Color.h>

#include <FL/Fl_Vk_Window.H>

namespace mrv
{
    namespace vulkan
    {
        using namespace tl;

        //! OpenGL Outline renderer.
        class Outline
        {
        public:
            Outline();
            ~Outline();

            void drawRect(
                VkCommandBuffer& cmd,
                const uint32_t frameIndex,
                Fl_Vk_Context& ctx,
                const math::Box2i&, const image::Color4f&,
                const math::Matrix4x4f& mvp);

        private:
            TLRENDER_PRIVATE();
        };

    } // namespace vulkan
} // namespace mrv
