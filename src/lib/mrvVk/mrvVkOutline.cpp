// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Mesh.h>

#include <tlVk/Mesh.h>
#include <tlVk/Util.h>
#include <tlVk/Shader.h>

#include "mrvVk/mrvVkOutline.h"

namespace tl
{
    namespace timeline_vk
    {
        extern std::string vertexSource();
        extern std::string meshFragmentSource();
    } // namespace timeline_vk
} // namespace tl

namespace mrv
{
    namespace vulkan
    {

        using namespace tl::vk;

        struct Outline::Private
        {
            std::shared_ptr<vk::Shader> shader;
            std::shared_ptr<vk::VBO> vbo;
            std::shared_ptr<vk::VAO> vao;
        };

        Outline::Outline() :
            _p(new Private)
        {
        }

        Outline::~Outline() {}

        void Outline::drawRect(
            Fl_Vk_Context& ctx, const math::Box2i& bbox,
            const image::Color4f& color, const math::Matrix4x4f& mvp)
        {
            TLRENDER_P();

            if (!p.shader)
            {
                // p.shader = Shader::create(
                //     timeline_vk::vertexSource(),
                //     timeline_vk::meshFragmentSource());
            }

            if (!p.vbo)
            {
                p.vbo = VBO::create(8, VBOType::Pos2_F32);
                p.vao.reset();
            }

            if (!p.vao)
            {
                p.vao = VAO::create(ctx);
            }

            p.shader->bind();
            p.shader->setUniform("color", color);
            p.shader->setUniform("transform.mvp", mvp);

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (p.vbo)
            {
                std::vector<uint8_t> vertexData;
                vertexData.resize(8 * 2 * sizeof(float));
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
            }
            if (p.vao && p.vbo)
            {
                p.vao->upload(p.vbo->getData());
                p.vao->bind();
                // p.vao->draw(GL_LINES, 0, p.vbo->getSize());
            }
        }

    } // namespace vulkan
} // namespace mrv
