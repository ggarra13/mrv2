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
    namespace timeline_vlk
    {
        extern std::string vertex2Source();
        extern std::string meshFragmentSource();
    } // namespace timeline_vlk
} // namespace tl

namespace mrv
{
    namespace vulkan
    {

        using namespace tl::vlk;

        struct Outline::Private
        {
            std::shared_ptr<vlk::Shader> shader;
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;
        };

        Outline::Outline() :
            _p(new Private)
        {
        }

        Outline::~Outline() {}

        void Outline::drawRect(
            VkCommandBuffer& cmd,
            const uint32_t frameIndex,
            Fl_Vk_Context& ctx, const math::Box2i& bbox,
            const image::Color4f& color, const math::Matrix4x4f& mvp)
        {
            TLRENDER_P();

            if (!p.shader)
            {
                p.shader = Shader::create(ctx,
                    timeline_vlk::vertex2Source(),
                    timeline_vlk::meshFragmentSource());
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

            p.shader->bind(frameIndex);
            p.shader->setUniform("transform.mvp", mvp, vlk::kShaderVertex);

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
                p.vao->bind(frameIndex);
                p.vao->draw(cmd, p.vbo);
            }
        }

    } // namespace vulkan
} // namespace mrv
