// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Matrix.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace gl
    {
        //! OpenGL shader.
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLRENDER_NON_COPYABLE(Shader);

        protected:
            void _init();

            Shader();

        public:
            ~Shader();

            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                const std::string& vertexSource,
                const std::string& fragmentSource);

            //! Get the vertex shader source.
            const std::string& getVertexSource() const;

            //! Get the fragment shader source.
            const std::string& getFragmentSource() const;

            //! Get the OpenGL shader program.
            unsigned int getProgram() const;

            //! Bind the shader.
            void bind();

            //! \name Uniforms
            //! Set uniform values.
            ///@{

            void setUniform(int, int);
            void setUniform(int, float);
            void setUniform(int, const math::Vector2f&);
            void setUniform(int, const math::Vector3f&);
            void setUniform(int, const math::Vector4f&);
            void setUniform(int, const math::Matrix3x3f&);
            void setUniform(int, const math::Matrix4x4f&);
            void setUniform(int, const image::Color4f&);
            void setUniform(int, const float[4]);

            void setUniform(int, const std::vector<int>&);
            void setUniform(int, const std::vector<float>&);
            void setUniform(int, const std::vector<math::Vector3f>&);
            void setUniform(int, const std::vector<math::Vector4f>&);

            void setUniform(const std::string&, int);
            void setUniform(const std::string&, float);
            void setUniform(const std::string&, const math::Vector2f&);
            void setUniform(const std::string&, const math::Vector3f&);
            void setUniform(const std::string&, const math::Vector4f&);
            void setUniform(const std::string&, const math::Matrix3x3f&);
            void setUniform(const std::string&, const math::Matrix4x4f&);
            void setUniform(const std::string&, const image::Color4f&);
            void setUniform(const std::string&, const float[4]);

            void setUniform(const std::string&, const std::vector<int>&);
            void setUniform(const std::string&, const std::vector<float>&);
            void
            setUniform(const std::string&, const std::vector<math::Vector3f>&);
            void
            setUniform(const std::string&, const std::vector<math::Vector4f>&);

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace gl
} // namespace tl
