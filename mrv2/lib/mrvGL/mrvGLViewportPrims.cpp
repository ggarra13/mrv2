// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <sstream>

#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

#include <Imath/ImathMatrix.h>

#include "mrvCore/mrvMesh.h"

#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLUtil.h"

namespace mrv
{

    void Viewport::_createCubicEnvironmentMap()
    {
        MRV2_GL();
        const auto& mesh = createEnvCube(1);
        const size_t numTriangles = mesh.triangles.size();
        if (!gl.vbo || (gl.vbo && gl.vbo->getSize() != numTriangles * 3))
        {
            gl.vbo =
                gl::VBO::create(numTriangles * 3, gl::VBOType::Pos3_F32_UV_U16);
            gl.vao.reset();
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl.vbo->getType()));
            CHECK_GL;
        }
        if (!gl.vao && gl.vbo)
        {
            gl.vao = gl::VAO::create(gl.vbo->getType(), gl.vbo->getID());
            CHECK_GL;
        }
    }

    void Viewport::_createSphericalEnvironmentMap()
    {
        TLRENDER_P();
        MRV2_GL();
        const auto mesh = geom::sphere(
            2.0F, p.environmentMapOptions.subdivisionX,
            p.environmentMapOptions.subdivisionY);
        const size_t numTriangles = mesh.triangles.size();
        if (!gl.vbo || (gl.vbo && gl.vbo->getSize() != numTriangles * 3))
        {
            gl.vbo =
                gl::VBO::create(numTriangles * 3, gl::VBOType::Pos3_F32_UV_U16);
            gl.vao.reset();
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl.vbo->getType()));
            CHECK_GL;
        }
        if (!gl.vao && gl.vbo)
        {
            gl.vao = gl::VAO::create(gl.vbo->getType(), gl.vbo->getID());
            CHECK_GL;
        }
    }

    math::Matrix4x4f Viewport::_createEnvironmentMap()
    {
        TLRENDER_P();

        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();

        const float PI = 3.141592654;
        const float DEG_TO_RAD = PI / 180.0;

        Imath::M44f vm;
        vm.makeIdentity();

        float rotX = p.environmentMapOptions.rotateX;
        float rotY = p.environmentMapOptions.rotateY;
        float fov = p.environmentMapOptions.focalLength;
        rotX *= DEG_TO_RAD;

        // Cubic environment map in OpenEXR is rotated -90 degrees
        if (p.environmentMapOptions.type == EnvironmentMapOptions::kCubic)
        {
            rotY -= 90;
        }
        
        rotY *= DEG_TO_RAD;
        fov *= DEG_TO_RAD;

        // Avoid gimbal lock
        Imath::V3f rotationX(rotX, 0, 0.F);
        vm = vm.rotate(rotationX);
        
        Imath::V3f rotationY(0, rotY, 0.F);
        vm = vm.rotate(rotationY);

        float aspect = viewportSize.w / (float)viewportSize.h;

        const float hAperture = p.environmentMapOptions.horizontalAperture;
        const float vAperture = p.environmentMapOptions.verticalAperture;
        float vAper = vAperture;
        if (vAper == 0.0F)
            vAper = hAperture * aspect;
        aspect = vAper / hAperture;

        float nearClip = 0.1F;
        float farClip = 3.F;

        // Create the perspective matrix
        Imath::M44f pm;
        memset(&pm, 0, sizeof(Imath::M44f));
        float tanHalfFov = tan(fov / 2.0f);
        pm[0][0] = 1.0f / (aspect * tanHalfFov);
        pm[1][1] = 1.0f / tanHalfFov;
        pm[2][2] = -(farClip + nearClip) / (farClip - nearClip);
        pm[2][3] = -1.0f;
        pm[3][2] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
        pm[3][3] = 0.0f;

        // Define your camera parameters
        Imath::V3f eye(0, 0, 1);
        Imath::V3f center(0, 0, -1);
        Imath::V3f up(0, 1, 0);

        // Create the lookAt matrix manually
        Imath::V3f f = (center - eye).normalized();
        Imath::V3f s = f.cross(up).normalized();
        Imath::V3f u = s.cross(f);

        Imath::M44f lookAtMatrix(
            s.x, s.y, s.z, 0.0f, u.x, u.y, u.z, 0.0f, -f.x, -f.y, -f.z, 0.0f,
            s.dot(eye), u.dot(eye), f.dot(eye), 1.0f);

        // Combine the perspective and look at matrices
        pm = lookAtMatrix * pm;

        // Add the rotation and scaling to the matrix
        const Imath::M44f vpm = vm * pm;

        auto mvp = math::Matrix4x4f(
            vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3], vpm[1][0], vpm[1][1],
            vpm[1][2], vpm[1][3], vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
            vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3]);
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
        MRV2_GL();

        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();

        const auto& mesh =
            geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
        const size_t numTriangles = mesh.triangles.size();
        if (!gl.vbo || (gl.vbo && gl.vbo->getSize() != numTriangles * 3))
        {
            gl.vbo =
                gl::VBO::create(numTriangles * 3, gl::VBOType::Pos2_F32_UV_U16);
            gl.vao.reset();
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
            CHECK_GL;
        }

        if (!gl.vao && gl.vbo)
        {
            gl.vao = gl::VAO::create(gl.vbo->getType(), gl.vbo->getID());
            CHECK_GL;
        }

        math::Matrix4x4f vm;
        vm =
            vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
        vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
        const auto pm = math::ortho(
            0.F, static_cast<float>(viewportSize.w), 0.F,
            static_cast<float>(viewportSize.h), -1.F, 1.F);

        return pm * vm;
    }

} // namespace mrv
