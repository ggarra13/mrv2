// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! Set and restore the render size.
        class RenderSizeState
        {
        public:
            RenderSizeState(const std::shared_ptr<IRender>&);

            ~RenderSizeState();

        private:
            TLRENDER_PRIVATE();
        };

        //! Set and restore the viewport.
        class ViewportState
        {
        public:
            ViewportState(const std::shared_ptr<IRender>&);

            ~ViewportState();

        private:
            TLRENDER_PRIVATE();
        };

        //! Set and restore whether the clipping rectangle is enabled.
        class ClipRectEnabledState
        {
        public:
            ClipRectEnabledState(const std::shared_ptr<IRender>&);

            ~ClipRectEnabledState();

        private:
            TLRENDER_PRIVATE();
        };

        //! Set and restore the clipping rectangle.
        class ClipRectState
        {
        public:
            ClipRectState(const std::shared_ptr<IRender>&);

            ~ClipRectState();

            const math::Box2i& getClipRect() const;

        private:
            TLRENDER_PRIVATE();
        };

        //! Set and restore the transform.
        class TransformState
        {
        public:
            TransformState(const std::shared_ptr<IRender>&);

            ~TransformState();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace timeline
} // namespace tl
