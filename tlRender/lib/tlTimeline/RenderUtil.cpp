// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timeline
    {
        struct RenderSizeState::Private
        {
            std::shared_ptr<IRender> render;
            math::Size2i size;
        };

        RenderSizeState::RenderSizeState(
            const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.size = render->getRenderSize();
        }

        RenderSizeState::~RenderSizeState()
        {
            TLRENDER_P();
            p.render->setRenderSize(p.size);
        }

        struct ViewportState::Private
        {
            std::shared_ptr<IRender> render;
            math::Box2i viewport;
        };

        ViewportState::ViewportState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.viewport = render->getViewport();
        }

        ViewportState::~ViewportState()
        {
            TLRENDER_P();
            p.render->setViewport(p.viewport);
        }

        struct ClipRectEnabledState::Private
        {
            std::shared_ptr<IRender> render;
            bool clipRectEnabled = false;
        };

        ClipRectEnabledState::ClipRectEnabledState(
            const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.clipRectEnabled = render->getClipRectEnabled();
        }

        ClipRectEnabledState::~ClipRectEnabledState()
        {
            TLRENDER_P();
            p.render->setClipRectEnabled(p.clipRectEnabled);
        }

        struct ClipRectState::Private
        {
            std::shared_ptr<IRender> render;
            math::Box2i clipRect;
        };

        ClipRectState::ClipRectState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.clipRect = render->getClipRect();
        }

        ClipRectState::~ClipRectState()
        {
            TLRENDER_P();
            p.render->setClipRect(p.clipRect);
        }

        const math::Box2i& ClipRectState::getClipRect() const
        {
            return _p->clipRect;
        }

        struct TransformState::Private
        {
            std::shared_ptr<IRender> render;
            math::Matrix4x4f transform;
        };

        TransformState::TransformState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.transform = render->getTransform();
        }

        TransformState::~TransformState()
        {
            TLRENDER_P();
            p.render->setTransform(p.transform);
        }
    } // namespace timeline
} // namespace tl
