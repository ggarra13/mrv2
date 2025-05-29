// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGLTest/ShaderTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Shader.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        ShaderTest::ShaderTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::ShaderTest", context)
        {
        }

        std::shared_ptr<ShaderTest>
        ShaderTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ShaderTest>(new ShaderTest(context));
        }

        void ShaderTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "ShaderTest", math::Size2i(1, 1), _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
                try
                {
                    auto shader = Shader::create(std::string(), std::string());
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
                try
                {
                    auto shader = Shader::create(

                        "#version 410\n"
                        "\n"
                        "in vec3 vPos;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = vec4(vPos, 1.0);\n"
                        "}\n",
                        std::string());
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
                try
                {
                    auto shader = Shader::create(
                        "#version 410\n"
                        "\n"
                        "in vec3 vPos;\n"
                        "\n"
                        "uniform int intValue;\n"
                        "uniform float floatValue;\n"
                        "uniform vec2 vec2Value;\n"
                        "uniform vec3 vec3Value;\n"
                        "uniform vec4 vec4Value;\n"
                        "uniform mat3 mat3Value;\n"
                        "uniform mat4 mat4Value;\n"
                        "uniform vec4 colorValue;\n"
                        "uniform vec4 color2Value;\n"
                        "uniform int intArray[3];\n"
                        "uniform float floatArray[3];\n"
                        "uniform vec3 vec3Array[3];\n"
                        "uniform vec4 vec4Array[3];\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = vec4(vPos, 1.0);\n"
                        "}\n",
                        "#version 410\n"
                        "\n"
                        "out vec4 fColor;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "\n"
                        "    fColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                        "}\n");
                    TLRENDER_ASSERT(!shader->getVertexSource().empty());
                    TLRENDER_ASSERT(!shader->getFragmentSource().empty());
                    TLRENDER_ASSERT(shader->getProgram());
                    shader->bind();

                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "intValue"),
                        1);
                    shader->setUniform(
                        glGetUniformLocation(
                            shader->getProgram(), "floatValue"),
                        1.F);
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec2Value"),
                        math::Vector2f(1.0, 2.0));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec2Value"),
                        math::Vector2f(1.0, 2.0));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec3Value"),
                        math::Vector3f(1.0, 2.0, 3.0));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec4Value"),
                        math::Vector4f(1.0, 2.0, 3.0, 4.0));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "mat3Value"),
                        math::Matrix3x3f());
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "mat4Value"),
                        math::Matrix4x4f());
                    shader->setUniform(
                        glGetUniformLocation(
                            shader->getProgram(), "colorValue"),
                        image::Color4f(0.F, .1F, .2F));
                    float color2[4] = {0.F, .1F, .2F, 1.F};
                    shader->setUniform(
                        glGetUniformLocation(
                            shader->getProgram(), "color2Value"),
                        color2);
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "intArray"),
                        std::vector<int>({1, 2, 3}));
                    shader->setUniform(
                        glGetUniformLocation(
                            shader->getProgram(), "floatArray"),
                        std::vector<float>({1.F, 2.F, 3.F}));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec3Array"),
                        std::vector<math::Vector3f>(
                            {math::Vector3f(1.F, 2.F, 3.F),
                             math::Vector3f(4.F, 5.F, 6.F),
                             math::Vector3f(7.F, 8.F, 9.F)}));
                    shader->setUniform(
                        glGetUniformLocation(shader->getProgram(), "vec4Array"),
                        std::vector<math::Vector4f>(
                            {math::Vector4f(1.F, 2.F, 3.F, 4.F),
                             math::Vector4f(5.F, 6.F, 7.F, 8.F),
                             math::Vector4f(9.F, 10.F, 11.F, 12.F)}));

                    shader->setUniform("intValue", 1);
                    shader->setUniform("floatValue", 1.F);
                    shader->setUniform("vec2Value", math::Vector2f(1.0, 2.0));
                    shader->setUniform("vec2Value", math::Vector2f(1.0, 2.0));
                    shader->setUniform(
                        "vec3Value", math::Vector3f(1.0, 2.0, 3.0));
                    shader->setUniform(
                        "vec4Value", math::Vector4f(1.0, 2.0, 3.0, 4.0));
                    shader->setUniform("mat3Value", math::Matrix3x3f());
                    shader->setUniform("mat4Value", math::Matrix4x4f());
                    shader->setUniform(
                        "colorValue", image::Color4f(0.F, .1F, .2F));
                    shader->setUniform("color2Value", color2);
                    shader->setUniform("intArray", std::vector<int>({1, 2, 3}));
                    shader->setUniform(
                        "floatArray", std::vector<float>({1.F, 2.F, 3.F}));
                    shader->setUniform(
                        "vec3Array", std::vector<math::Vector3f>(
                                         {math::Vector3f(1.F, 2.F, 3.F),
                                          math::Vector3f(4.F, 5.F, 6.F),
                                          math::Vector3f(7.F, 8.F, 9.F)}));
                    shader->setUniform(
                        "vec4Array",
                        std::vector<math::Vector4f>(
                            {math::Vector4f(1.F, 2.F, 3.F, 4.F),
                             math::Vector4f(5.F, 6.F, 7.F, 8.F),
                             math::Vector4f(9.F, 10.F, 11.F, 12.F)}));
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }
    } // namespace gl_tests
} // namespace tl
