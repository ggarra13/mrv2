// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_Vk_Utils.H>

#include <iostream>

namespace tl
{
    namespace vk
    {
        struct Shader::Private
        {
            std::string vertexSource;
            std::string fragmentSource;
            VkShaderModule vertex = VK_NULL_HANDLE;
            VkShaderModule fragment = VK_NULL_HANDLE;
        };

        void Shader::_init()
        {
            TLRENDER_P();

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.vertexSource,
                    shaderc_vertex_shader, // Shader type
                    "vertex_shader.glsl"   // Filename for error reporting
                );

                p.vertex = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                p.vertex = VK_NULL_HANDLE;
            }

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.fragmentSource,
                    shaderc_fragment_shader, // Shader type
                    "frag_shader.glsl"       // Filename for error reporting
                );

                // Assuming you have a VkDevice 'device' already created
                p.fragment = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                p.fragment = VK_NULL_HANDLE;
            }
        }

        Shader::Shader(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        Shader::~Shader()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            if (p.fragment != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.fragment, nullptr);

            if (p.vertex != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.vertex, nullptr);
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx, const std::string& vertexSource,
            const std::string& fragmentSource)
        {
            auto out = std::shared_ptr<Shader>(new Shader(ctx));
            out->_p->vertexSource = vertexSource;
            out->_p->fragmentSource = fragmentSource;
            out->_init();
            return out;
        }

        const VkShaderModule& Shader::getVertex() const
        {
            return _p->vertex;
        }

        const VkShaderModule& Shader::getFragment() const
        {
            return _p->fragment;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _p->vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _p->fragmentSource;
        }

        void Shader::bind()
        {
            // glUseProgram(_p->program);
        }

        void Shader::setUniform(int location, int value)
        {
            // glUniform1i(location, value);
        }

        void Shader::setUniform(int location, float value)
        {
            // glUniform1f(location, value);
        }

        void Shader::setUniform(int location, const math::Vector2f& value)
        {
            // glUniform2fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Vector3f& value)
        {
            // glUniform3fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Vector4f& value)
        {
            // glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Matrix3x3f& value)
        {
            // glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(int location, const math::Matrix4x4f& value)
        {
            // glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(int location, const image::Color4f& value)
        {
            // glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(int location, const float value[4])
        {
            // glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(int location, const std::vector<int>& value)
        {
            // glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(int location, const std::vector<float>& value)
        {
            // glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            int location, const std::vector<math::Vector3f>& value)
        {
            // glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(
            int location, const std::vector<math::Vector4f>& value)
        {
            // glUniform4fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(const std::string& name, int value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform1i(location, value);
        }

        void Shader::setUniform(const std::string& name, float value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform1f(location, value);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector2f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform2fv(location, 1, &value.x);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector3f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform3fv(location, 1, &value.x);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector4f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(
            const std::string& name, const math::Matrix3x3f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(
            const std::string& name, const math::Matrix4x4f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }

        void
        Shader::setUniform(const std::string& name, const image::Color4f& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(const std::string& name, const float value[4])
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<int>& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<float>& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<math::Vector3f>& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<math::Vector4f>& value)
        {
            // const GLint location =
            //     glGetUniformLocation(_p->program, name.c_str());
            // glUniform4fv(location, value.size(), &value[0].x);
        }
    } // namespace vk
} // namespace tl
