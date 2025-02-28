// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <tlCore/StringFormat.h>

#include <tlGL/Shader.h>
#include <tlGL/Mesh.h>
#include <tlTimelineGL/RenderPrivate.h>
#include <tlGL/Util.h>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLShaders.h"
#include "mrvGL/mrvGLUtil.h"

namespace
{
    const char* kModule = "glutil";
}

namespace mrv
{
    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const image::Color4f& color, const bool soft)
    {
        geom::TriangleMesh2 mesh;
        const int CIRCLE_SEGMENTS = 32;
        const double twoPi = math::pi * 2.0;
        math::Vector2f v(center.x, center.y);
        mesh.v.push_back(v);
        for (int i = 0; i < CIRCLE_SEGMENTS; ++i)
        {
            v.x = center.x + radius * std::cos(twoPi * i / CIRCLE_SEGMENTS);
            v.y = center.y + radius * std::sin(twoPi * i / CIRCLE_SEGMENTS);
            mesh.v.push_back(v);
        }

        geom::Triangle2 triangle;
        unsigned numTriangles = CIRCLE_SEGMENTS;
        int i;
        for (i = 1; i < numTriangles; ++i)
        {
            triangle.v[0] = 1;
            triangle.v[1] = i + 1;
            triangle.v[2] = i + 2;
            mesh.triangles.emplace_back(triangle);
        }

        triangle.v[0] = 1;
        triangle.v[1] = 2;
        triangle.v[2] = i + 1;
        mesh.triangles.emplace_back(triangle);

        math::Vector2i pos;
        render->drawMesh(mesh, pos, color);
    }

    geom::TriangleMesh2
    checkers(const math::Box2i& box, const math::Size2i& checkerSize)
    {
        geom::TriangleMesh2 out;

        // X points.
        std::vector<int> xs;
        int x = box.min.x;
        for (; x < box.max.x; x += checkerSize.w)
        {
            xs.push_back(x);
        }
        if (x >= box.max.x)
        {
            xs.push_back(box.max.x);
        }

        // Y points.
        std::vector<int> ys;
        int y = box.min.y;
        for (; y < box.max.y; y += checkerSize.h)
        {
            ys.push_back(y);
        }
        if (y >= box.max.y)
        {
            ys.push_back(box.max.y);
        }

        if (!xs.empty() && !ys.empty())
        {
            // 2D points.
            for (int j = 0; j < ys.size(); ++j)
            {
                for (int i = 0; i < xs.size(); ++i)
                {
                    out.v.push_back(math::Vector2f(xs[i], ys[j]));
                }
            }

            // Triangles.
            for (int j = 0; j < ys.size() - 1; j += 2)
            {
                for (int i = 0; i < xs.size() - 1; i += 2)
                {
                    const int v0 = j * xs.size() + i + 1;
                    const int v1 = j * xs.size() + (i + 1) + 1;
                    const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                    const int v3 = (j + 1) * xs.size() + i + 1;
                    out.triangles.push_back(
                        {geom::Vertex2(v0, 0, 0), geom::Vertex2(v1, 0, 0),
                         geom::Vertex2(v2, 0, 0)});
                    out.triangles.push_back(
                        {geom::Vertex2(v2, 0, 0), geom::Vertex2(v3, 0, 0),
                         geom::Vertex2(v0, 0, 0)});
                }
            }
            for (int j = 1; j < ys.size() - 1; j += 2)
            {
                for (int i = 1; i < xs.size() - 1; i += 2)
                {
                    const int v0 = j * xs.size() + i + 1;
                    const int v1 = j * xs.size() + (i + 1) + 1;
                    const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                    const int v3 = (j + 1) * xs.size() + i + 1;
                    out.triangles.push_back(
                        {geom::Vertex2(v0, 0, 0), geom::Vertex2(v1, 0, 0),
                         geom::Vertex2(v2, 0, 0)});
                    out.triangles.push_back(
                        {geom::Vertex2(v2, 0, 0), geom::Vertex2(v3, 0, 0),
                         geom::Vertex2(v0, 0, 0)});
                }
            }
        }

        return out;
    }

    // Function to perform bilinear interpolation for resizing
    void resizeImage(
        GLubyte* targetPixels, GLubyte* srcPixels, const int srcWidth,
        const int srcHeight, const int targetWidth, const int targetHeight)
    {

        // Calculate scaling factors
        double scaleX = static_cast<double>(srcWidth) / targetWidth;
        double scaleY = static_cast<double>(srcHeight) / targetHeight;

        for (int y = 0; y < targetHeight; ++y)
        {
            for (int x = 0; x < targetWidth; ++x)
            {
                // Calculate coordinates in the source image
                double srcX = x * scaleX;
                double srcY = y * scaleY;

                // Calculate indices of the nearest pixels in the source image
                int x1 = static_cast<int>(srcX);
                int y1 = static_cast<int>(srcY);
                int x2 = std::min(x1 + 1, srcWidth - 1);
                int y2 = std::min(y1 + 1, srcHeight - 1);

                // Calculate interpolation weights
                double wx2 = srcX - x1;
                double wy2 = srcY - y1;
                double wx1 = 1.0 - wx2;
                double wy1 = 1.0 - wy2;

                // Get the pixels from the source image
                GLubyte* p11 = &srcPixels[(y1 * srcWidth + x1) * 4];
                GLubyte* p12 = &srcPixels[(y2 * srcWidth + x1) * 4];
                GLubyte* p21 = &srcPixels[(y1 * srcWidth + x2) * 4];
                GLubyte* p22 = &srcPixels[(y2 * srcWidth + x2) * 4];

                // Perform bilinear interpolation for each channel
                GLubyte* targetPixel = &targetPixels[(y * targetWidth + x) * 4];
                targetPixel[0] = static_cast<GLubyte>(
                    wx1 * wy1 * p11[0] + wx2 * wy1 * p21[0] +
                    wx1 * wy2 * p12[0] + wx2 * wy2 * p22[0]);
                targetPixel[1] = static_cast<GLubyte>(
                    wx1 * wy1 * p11[1] + wx2 * wy1 * p21[1] +
                    wx1 * wy2 * p12[1] + wx2 * wy2 * p22[1]);
                targetPixel[2] = static_cast<GLubyte>(
                    wx1 * wy1 * p11[2] + wx2 * wy1 * p21[2] +
                    wx1 * wy2 * p12[2] + wx2 * wy2 * p22[2]);
                targetPixel[3] = static_cast<GLubyte>(
                    wx1 * wy1 * p11[3] + wx2 * wy1 * p21[3] +
                    wx1 * wy2 * p12[3] + wx2 * wy2 * p22[3]);
            }
        }
    }
} // namespace mrv
