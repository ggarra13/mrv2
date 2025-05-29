// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGLTest/MeshTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Mesh.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        MeshTest::MeshTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::MeshTest", context)
        {
        }

        std::shared_ptr<MeshTest>
        MeshTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MeshTest>(new MeshTest(context));
        }

        void MeshTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "MeshTest", math::Size2i(1, 1), _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
                _enums();
                _convert();
                _mesh();
            }
        }

        void MeshTest::_enums()
        {
            _enum<VBOType>("VBOType", getVBOTypeEnums);
            for (auto i : getVBOTypeEnums())
            {
                _print(string::Format("{0} byte count: {1}")
                           .arg(getLabel(i))
                           .arg(getByteCount(i)));
            }
        }

        void MeshTest::_convert()
        {
            for (auto type :
                 {VBOType::Pos2_F32, VBOType::Pos2_F32_UV_U16,
                  VBOType::Pos2_F32_Color_F32})
            {
                auto mesh = geom::box(math::Box2f(0.F, 1.F, 2.F, 3.F));
                auto data = convert(mesh, type);
                TLRENDER_ASSERT(!data.empty());
            }
            for (auto type :
                 {VBOType::Pos3_F32, VBOType::Pos3_F32_UV_U16,
                  VBOType::Pos3_F32_UV_U16_Normal_U10,
                  VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8,
                  VBOType::Pos3_F32_UV_F32_Normal_F32,
                  VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32,
                  VBOType::Pos3_F32_Color_U8})
            {
                auto mesh = geom::sphere(10.F, 10, 10);
                auto data = convert(mesh, type);
                TLRENDER_ASSERT(!data.empty());
            }
        }

        void MeshTest::_mesh()
        {
            for (auto type :
                 {VBOType::Pos2_F32, VBOType::Pos2_F32_UV_U16,
                  VBOType::Pos2_F32_Color_F32})
            {
                auto mesh = geom::box(math::Box2f(0.F, 1.F, 2.F, 3.F));
                auto data = convert(mesh, type);
                auto vbo = VBO::create(mesh.v.size(), type);
                TLRENDER_ASSERT(mesh.v.size() == vbo->getSize());
                TLRENDER_ASSERT(type == vbo->getType());
                TLRENDER_ASSERT(vbo->getID());
                vbo->copy(data);
                vbo->copy(data, 0, 1);
                auto vao = VAO::create(type, vbo->getID());
                TLRENDER_ASSERT(vao->getID());
                vao->bind();
                vao->draw(GL_TRIANGLES, 0, vbo->getSize());
            }
            for (auto type :
                 {VBOType::Pos3_F32, VBOType::Pos3_F32_UV_U16,
                  VBOType::Pos3_F32_UV_U16_Normal_U10,
                  VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8,
                  VBOType::Pos3_F32_UV_F32_Normal_F32,
                  VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32,
                  VBOType::Pos3_F32_Color_U8})
            {
                auto mesh = geom::sphere(10.F, 10, 10);
                auto data = convert(mesh, type);
                auto vbo = VBO::create(mesh.v.size(), type);
                TLRENDER_ASSERT(mesh.v.size() == vbo->getSize());
                TLRENDER_ASSERT(type == vbo->getType());
                TLRENDER_ASSERT(vbo->getID());
                vbo->copy(data);
                vbo->copy(data, 0, 1);
                auto vao = VAO::create(type, vbo->getID());
                TLRENDER_ASSERT(vao->getID());
                vao->bind();
                vao->draw(GL_TRIANGLES, 0, vbo->getSize());
            }
        }
    } // namespace gl_tests
} // namespace tl
