// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWGPU/mrvWGPUViewport.h"
#include "mrvWGPU/mrvWGPUViewportPrivate.h"

#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvCore/mrvMesh.h"

#include <tlWGPU/Mesh.h>

#include <tlCore/Mesh.h>

namespace mrv
{

    namespace webgpu
    {

        void Viewport::_createCubicEnvironmentMap()
        {
            MRV2_WGPU();
            const auto& mesh = createEnvCube(1, true);
            const size_t numTriangles = mesh.triangles.size();
            if (!wgpu.vbo || (wgpu.vbo && wgpu.vbo->getSize() != numTriangles * 3))
            {
                // \@bug: wgpu:
                // wait_device();
                wgpu.vbo = wgpu::VBO::create(
                    numTriangles * 3, wgpu::VBOType::Pos3_F32_UV_U16);
                wgpu.vao.reset();
            }
            if (wgpu.vbo)
            {
                wgpu.vbo->copy(convert(mesh, wgpu.vbo->getType()));
            }
            if (!wgpu.vao && wgpu.vbo)
            {
                wgpu.vao = wgpu::VAO::create(ctx);
                prepare_pipeline();
            }
        }

        void Viewport::_createSphericalEnvironmentMap()
        {
            TLRENDER_P();
            MRV2_WGPU();
            const auto mesh = geom::sphere(
                2.0F, p.environmentMapOptions.subdivisionX,
                p.environmentMapOptions.subdivisionY);
            const size_t numTriangles = mesh.triangles.size();
            if (!wgpu.vbo || (wgpu.vbo && wgpu.vbo->getSize() != numTriangles * 3))
            {
                // \@bug: wgpu:
                // wait_device();
                wgpu.vbo = wgpu::VBO::create(
                    numTriangles * 3, wgpu::VBOType::Pos3_F32_UV_U16);
                wgpu.vao.reset();
            }
            if (wgpu.vbo)
            {
                wgpu.vbo->copy(convert(mesh, wgpu.vbo->getType()));
            }
            if (!wgpu.vao && wgpu.vbo)
            {
                wgpu.vao = wgpu::VAO::create(ctx);
                prepare_pipeline();
            }
        }

        math::Matrix4x4f Viewport::_createEnvironmentMap()
        {
            TLRENDER_P();

            const auto& renderSize = getRenderSize();
            const auto& viewportSize = getViewportSize();

            float rotX = p.environmentMapOptions.rotateX;
            float rotY = p.environmentMapOptions.rotateY;

            // Cubic environment map in OpenEXR is rotated -90 degrees
            if (p.environmentMapOptions.type == EnvironmentMapOptions::kCubic)
            {
                rotY -= 90.0F;
            }

            // tlRender's perspective functions expects fov in degrees
            float fov = p.environmentMapOptions.focalLength;

            // 1. View/Model Matrix
            // Because tlRender uses column-vector math (Matrix * Vector), the transformation
            // order is reversed compared to Imath's row-vector layout.
            // Imath: vm.rotate(rotX).rotate(rotY) -> tlRender: rotateY * rotateX
            auto vm = math::rotateY(rotY) * math::rotateX(rotX);

            // Aspect ratio calculation remains mostly unchanged
            float aspect = viewportSize.w / static_cast<float>(viewportSize.h);

            const float hAperture = p.environmentMapOptions.horizontalAperture;
            const float vAperture = p.environmentMapOptions.verticalAperture;
            float vAper = vAperture;
            if (vAper == 0.0F)
            {
                vAper = hAperture * aspect;
            }
            aspect = vAper / hAperture;

            float nearClip = 0.1F;
            float farClip = 3.0F;

            // 2. Perspective Matrix
            // tlRender  provides a built-in perspective function, removing the need
            // to manually populate the tanHalfFov projection matrix elements.
            auto pm = math::perspective(fov, aspect, nearClip, farClip);

            // 3. LookAt Matrix
            // The original manual lookAt parameters (eye: 0,0,1, center: 0,0,-1, up: 0,1,0)
            // evaluate mathematically to a simple translation matrix of -1 on the Z axis.
            auto lookAtMatrix = math::translate(math::Vector3f(0.0F, 0.0F, -1.0F));

            // 4. Combine Matrices
            // Apply the lookAt offset to the projection matrix.
            pm = pm * lookAtMatrix;

            // Final Model-View-Projection matrix.
            // Note the reversed multiplication order (pm * vm) for column-vectors.
            // We no longer need the 16-element manual copy, as we're already using Matrix4x4f natively.
            const auto mvp = pm * vm;

            switch (p.environmentMapOptions.type)
            {
            case EnvironmentMapOptions::kSpherical:
                _createSphericalEnvironmentMap();
                break;
            case EnvironmentMapOptions::kCubic:
                _createCubicEnvironmentMap();
                break;
            default:
                throw std::runtime_error("Invalid EnvionmentMap type");
            }
            return mvp;
        }

        math::Matrix4x4f Viewport::_createTexturedRectangle()
        {
            TLRENDER_P();
            MRV2_WGPU();

            const auto& renderSize = getRenderSize();

            const bool flipV = true;
            const auto& mesh =
                geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h), flipV);
            const size_t numTriangles = mesh.triangles.size();
            if (!wgpu.vbo || (wgpu.vbo && wgpu.vbo->getSize() != numTriangles * 3))
            {
                // \@bug: how to do this
                // wait_device();
                wgpu.vbo = wgpu::VBO::create(
                    numTriangles * 3, wgpu::VBOType::Pos2_F32_UV_U16);
                wgpu.vao.reset();
            }
            if (wgpu.vbo)
            {
                wgpu.vbo->copy(convert(mesh, wgpu::VBOType::Pos2_F32_UV_U16));
            }

            if (!wgpu.vao && wgpu.vbo)
            {
                wgpu.vao = wgpu::VAO::create(ctx);
                prepare_pipeline();
            }

            return projectionMatrix();
        }

    } // namespace vulkan

} // namespace mrv
