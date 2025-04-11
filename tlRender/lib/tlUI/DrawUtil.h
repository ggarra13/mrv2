// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Image.h>
#include <tlCore/Mesh.h>

namespace tl
{
    namespace ui
    {
        //! Create a mesh for drawing a rectangle.
        geom::TriangleMesh2
        rect(const math::Box2i&, int cornerRadius = 0, size_t resolution = 8);

        //! Create a mesh for drawing a circle.
        geom::TriangleMesh2
        circle(const math::Vector2i&, int radius = 0, size_t resolution = 120);

        //! Create a mesh for drawing a border.
        geom::TriangleMesh2 border(
            const math::Box2i&, int width, int radius = 0,
            size_t resolution = 8);

        //! Create a mesh for drawing a shadow.
        geom::TriangleMesh2 shadow(
            const math::Box2i&, int cornerRadius, const float alpha = .2F,
            size_t resolution = 8);
    } // namespace ui
} // namespace tl
