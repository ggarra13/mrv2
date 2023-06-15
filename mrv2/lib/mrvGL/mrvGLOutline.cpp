// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>
#include <tlGL/Shader.h>

#include <tlGlad/gl.h>

#include "mrvGLOutline.h"

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
            const math::BBox2i& bbox, const imaging::Color4f& color,
            const math::Matrix4x4f& mvp)
        {
            TLRENDER_P();

            if (!p.shader)
            {
                p.shader = Shader::create(
                    timeline::vertexSource(), timeline::meshFragmentSource());
            }

            if (!p.vbo)
            {
                p.vbo = VBO::create(2 * 3, VBOType::Pos2_F32);
            }

            if (!p.vao)
            {
                p.vao = VAO::create(p.vbo->getType(), p.vbo->getID());
            }

            p.shader->bind();
            p.shader->setUniform("color", color);
            p.shader->setUniform("transform.mvp", mvp);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (p.vbo)
            {
                p.vbo->copy(convert(geom::bbox(bbox), p.vbo->getType()));
            }
            if (p.vao)
            {
                p.vao->bind();
                p.vao->draw(GL_LINE_LOOP, 0, p.vbo->getSize());
            }
        }

    } // namespace gl
} // namespace tl
