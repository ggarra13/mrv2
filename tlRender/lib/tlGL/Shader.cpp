// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Shader.h>

#include <tlGL/GL.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

namespace tl
{
    namespace gl
    {
        struct Shader::Private
        {
            std::string vertexSource;
            std::string fragmentSource;
            GLuint vertex = 0;
            GLuint fragment = 0;
            GLuint program = 0;
        };

        void Shader::_init()
        {
            TLRENDER_P();

            p.vertex = glCreateShader(GL_VERTEX_SHADER);
            if (!p.vertex)
            {
                throw std::runtime_error("Cannot create vertex shader");
            }
            const char* src = p.vertexSource.c_str();
            glShaderSource(p.vertex, 1, &src, NULL);
            glCompileShader(p.vertex);
            int success = 0;
            glGetShaderiv(p.vertex, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetShaderInfoLog(
                    p.vertex, string::cBufferSize, NULL, infoLog);
                auto lines = string::split(
                    p.vertexSource, {'\n', '\r'},
                    string::SplitOptions::KeepEmpty);
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    lines[i].insert(0, string::Format("{0}: ").arg(i));
                }
                lines.push_back(infoLog);
                // std::cout << string::join(lines, '\n') << std::endl;
                throw std::runtime_error(string::join(lines, '\n'));
            }

            p.fragment = glCreateShader(GL_FRAGMENT_SHADER);
            if (!p.fragment)
            {
                throw std::runtime_error("Cannot create fragment shader");
            }
            src = p.fragmentSource.c_str();
            glShaderSource(p.fragment, 1, &src, NULL);
            glCompileShader(p.fragment);
            glGetShaderiv(p.fragment, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetShaderInfoLog(
                    p.fragment, string::cBufferSize, NULL, infoLog);
                auto lines = string::split(
                    p.fragmentSource, {'\n', '\r'},
                    string::SplitOptions::KeepEmpty);
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    lines[i].insert(0, string::Format("{0}: ").arg(i));
                }
                lines.push_back(infoLog);
                // std::cout << string::join(lines, '\n') << std::endl;
                throw std::runtime_error(string::join(lines, '\n'));
            }

            p.program = glCreateProgram();
            glAttachShader(p.program, p.vertex);
            glAttachShader(p.program, p.fragment);
            glLinkProgram(p.program);
            glGetProgramiv(p.program, GL_LINK_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetProgramInfoLog(
                    p.program, string::cBufferSize, NULL, infoLog);
                throw std::runtime_error(infoLog);
            }
        }

        Shader::Shader() :
            _p(new Private)
        {
        }

        Shader::~Shader()
        {
            TLRENDER_P();
            if (p.program)
            {
                glDeleteProgram(p.program);
                p.program = 0;
            }
            if (p.vertex)
            {
                glDeleteShader(p.vertex);
                p.vertex = 0;
            }
            if (p.fragment)
            {
                glDeleteShader(p.fragment);
                p.fragment = 0;
            }
        }

        std::shared_ptr<Shader> Shader::create(
            const std::string& vertexSource, const std::string& fragmentSource)
        {
            auto out = std::shared_ptr<Shader>(new Shader);
            out->_p->vertexSource = vertexSource;
            out->_p->fragmentSource = fragmentSource;
            out->_init();
            return out;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _p->vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _p->fragmentSource;
        }

        unsigned int Shader::getProgram() const
        {
            return _p->program;
        }

        void Shader::bind()
        {
            glUseProgram(_p->program);
        }

        void Shader::setUniform(int location, int value)
        {
            glUniform1i(location, value);
        }

        void Shader::setUniform(int location, float value)
        {
            glUniform1f(location, value);
        }

        void Shader::setUniform(int location, const math::Vector2f& value)
        {
            glUniform2fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Vector3f& value)
        {
            glUniform3fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Vector4f& value)
        {
            glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(int location, const math::Matrix3x3f& value)
        {
            glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(int location, const math::Matrix4x4f& value)
        {
            glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(int location, const image::Color4f& value)
        {
            glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(int location, const float value[4])
        {
            glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(int location, const std::vector<int>& value)
        {
            glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(int location, const std::vector<float>& value)
        {
            glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            int location, const std::vector<math::Vector3f>& value)
        {
            glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(
            int location, const std::vector<math::Vector4f>& value)
        {
            glUniform4fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(const std::string& name, int value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform1i(location, value);
        }

        void Shader::setUniform(const std::string& name, float value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform1f(location, value);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector2f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform2fv(location, 1, &value.x);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector3f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform3fv(location, 1, &value.x);
        }

        void
        Shader::setUniform(const std::string& name, const math::Vector4f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(
            const std::string& name, const math::Matrix3x3f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(
            const std::string& name, const math::Matrix4x4f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }

        void
        Shader::setUniform(const std::string& name, const image::Color4f& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(const std::string& name, const float value[4])
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<int>& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<float>& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<math::Vector3f>& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(
            const std::string& name, const std::vector<math::Vector4f>& value)
        {
            const GLint location =
                glGetUniformLocation(_p->program, name.c_str());
            glUniform4fv(location, value.size(), &value[0].x);
        }
    } // namespace gl
} // namespace tl
