// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>
#include <tlGL/Shader.h>

#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLOutline.h"

namespace tl
{
    namespace timeline
    {
        extern std::string vertexSource();
        extern std::string meshFragmentSource();
    } // namespace timeline

    namespace gl
    {

        struct Outline::Private
        {
            std::shared_ptr<Shader> shader;
            std::shared_ptr<VBO> vbo;
            std::shared_ptr<VAO> vao;
        };

        Outline::Outline() :
            _p(new Private)
        {
        }

        Outline::~Outline() {}

        void Outline::drawRect(
            const math::Box2i& bbox, const image::Color4f& color,
            const math::Matrix4x4f& mvp)
        {
            TLRENDER_P();

            if (!p.shader)
            {
                p.shader = Shader::create(
                    timeline::vertexSource(), timeline::meshFragmentSource());
                CHECK_GL;
            }

            if (!p.vbo)
            {
                p.vbo = VBO::create(8, VBOType::Pos2_F32);
                p.vao.reset();
                CHECK_GL;
            }

            if (!p.vao)
            {
                p.vao = VAO::create(p.vbo->getType(), p.vbo->getID());
                CHECK_GL;
            }

            p.shader->bind();
            CHECK_GL;
            p.shader->setUniform("color", color);
            CHECK_GL;
            p.shader->setUniform("transform.mvp", mvp);
            CHECK_GL;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            CHECK_GL;

            if (p.vbo)
            {
                std::vector<uint8_t> vertexData;
                vertexData.resize(8 * 2 * sizeof(GLfloat));
                float* v = reinterpret_cast<float*>(vertexData.data());
                v[0] = bbox.min.x;
                v[1] = bbox.min.y;
                v[2] = bbox.max.x;
                v[3] = bbox.min.y;

                v[4] = bbox.max.x;
                v[5] = bbox.min.y;
                v[6] = bbox.max.x;
                v[7] = bbox.max.y;

                v[8] = bbox.max.x;
                v[9] = bbox.max.y;
                v[10] = bbox.min.x;
                v[11] = bbox.max.y;

                v[12] = bbox.min.x;
                v[13] = bbox.max.y;
                v[14] = bbox.min.x;
                v[15] = bbox.min.y;
                p.vbo->copy(vertexData);
                CHECK_GL;
            }
            if (p.vao && p.vbo)
            {
                p.vao->bind();
                CHECK_GL;
                p.vao->draw(GL_LINES, 0, p.vbo->getSize());
                CHECK_GL;
            }
        }

    } // namespace gl
} // namespace tl
