// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

#include <tlGlad/gl.h>

#include "mrvCore/mrvMesh.h"

#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLUtil.h"

#include <glm/gtc/matrix_transform.hpp>

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
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl::VBOType::Pos3_F32_UV_U16));
            CHECK_GL;
        }
        if (!gl.vao && gl.vbo)
        {
            gl.vao =
                gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, gl.vbo->getID());
            CHECK_GL;
        }
    }

    void Viewport::_createSphericalEnvironmentMap()
    {
        TLRENDER_P();
        MRV2_GL();
        const auto& mesh = geom::createSphere(
            2.0F, p.environmentMapOptions.subdivisionX,
            p.environmentMapOptions.subdivisionY);
        const size_t numTriangles = mesh.triangles.size();
        if (!gl.vbo || (gl.vbo && gl.vbo->getSize() != numTriangles * 3))
        {
            gl.vbo =
                gl::VBO::create(numTriangles * 3, gl::VBOType::Pos3_F32_UV_U16);
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl::VBOType::Pos3_F32_UV_U16));
            CHECK_GL;
        }
        if (!gl.vao && gl.vbo)
        {
            gl.vao =
                gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, gl.vbo->getID());
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

        glm::mat4x4 vm(1.F);
        float rotX = p.environmentMapOptions.rotateX;
        float rotY = p.environmentMapOptions.rotateY;
        float fov = p.environmentMapOptions.focalLength;
        fov *= DEG_TO_RAD;
        const float hAperture = p.environmentMapOptions.horizontalAperture;
        const float vAperture = p.environmentMapOptions.verticalAperture;
        if (p.environmentMapOptions.type == EnvironmentMapOptions::kCubic)
        {
            vm = glm::scale(vm, glm::vec3(1, -1, 1));
            rotY += 90;
        }
        rotX *= DEG_TO_RAD;
        rotY *= DEG_TO_RAD;
        vm = glm::rotate(vm, rotX, glm::vec3(1, 0, 0));
        vm = glm::rotate(vm, rotY, glm::vec3(0, 1, 0));

        float aspect = viewportSize.w / (float)viewportSize.h;
        float remderSspect = renderSize.w / (float)renderSize.h;

        float vAper = vAperture;
        if (vAper == 0.0F)
            vAper = hAperture * aspect;
        aspect = vAper / hAperture;

        glm::mat4x4 pm = glm::perspective(fov, aspect, 0.1F, 3.F);
        pm = pm *
             glm::lookAt(
                 glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
        glm::mat4x4 vpm = pm * vm;
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
            CHECK_GL;
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
            CHECK_GL;
        }

        if (!gl.vao && gl.vbo)
        {
            gl.vao =
                gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, gl.vbo->getID());
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
