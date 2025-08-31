
#include <tlTimelineVk/RenderPrivate.h>
#include <tlTimelineVk/RenderStructs.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace timeline_vlk
    {
        void Render::drawStereo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const StereoType stereoType,
            const float eyeSeparation,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            const VkColorComponentFlags rgbaMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
            const VkColorComponentFlags noneMask[] = { 0 };
            
            
            image::Color4f color(1.F, 0.F, 0.F);
            VkPipelineLayout pipelineLayout;
            std::string pipelineLayoutName = "stereo1";
            const math::Size2i& offscreenBufferSize = p.fbo->getSize();
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
            offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
            offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
            offscreenBufferOptions.clear = false;
            if (doCreate(p.buffers["stereo_image"], offscreenBufferSize,
                         offscreenBufferOptions))
            {
                p.buffers["stereo_image"] =
                    vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                 offscreenBufferOptions);
            }
            

            // Main FBO Transitions
            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);
            

            // Draw left image to "stereo_image" buffer
            if (!videoData.empty() && !boxes.empty())
            {
                p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
                
                _drawVideo(
                    p.buffers["stereo_image"], "display", 
                    videoData[0], boxes[0],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                
                p.buffers["stereo_image"]->transitionToShaderRead(p.cmd);
            }

            math::Size2i size;
            if (!boxes.empty())
            {
                size.w = boxes[0].w();
                size.h = boxes[0].h();
            }

            // Create stencil triangle mesh (scanlines, columns or dots)
            geom::TriangleMesh2 mesh;
            if (stereoType == StereoType::kScanlines)
            {
                mesh = geom::scanlines(0, size);
            }
            else
            {
                mesh = geom::columns(0, size);
            }
            
            p.vbos["stereo"] =
                vlk::VBO::create(mesh.triangles.size() * 3,
                                 vlk::VBOType::Pos2_F32);
            p.vaos["stereo"] = vlk::VAO::create(ctx);
            
            p.vbos["stereo"]->copy(convert(mesh,
                                           p.vbos["stereo"]->getType()));

            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);

            // ----- FIRST RENDER PASS OF LEFT VIDEO
            p.fbo->beginClearRenderPass(p.cmd);

            pipelineLayoutName = "stereo1_stencil";
            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_FALSE;
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;
                ds.stencilTestEnable = VK_TRUE;
            
                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_REPLACE;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0xFF;
                stencilOp.reference = 1;
                ds.front = ds.back = stencilOp;

                //
                // These are for dynamic stencils
                //
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_REPLACE,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_ALWAYS);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0xFF);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);

                // Draw stencil mask
                createPipeline("stereo1_stencil", pipelineLayoutName,
                               p.fbo->getClearRenderPass(),
                               p.shaders["wipe"], p.vbos["stereo"],
                               cb, ds);
            }
            
            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            
            _createBindingSet(p.shaders["wipe"]);
            color = image::Color4f(0.F, 1.F, 0.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               p.shaders["wipe"]->getPushStageFlags(), 0,
                               sizeof(color), &color);
            p.shaders["wipe"]->setUniform("transform.mvp", p.transform,
                                          vlk::kShaderVertex);
            _bindDescriptorSets(pipelineLayoutName, "wipe");

            // If I draw with colors, the pattern is being drawn.
            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, noneMask);
            
            _vkDraw("stereo");

            // Draw video
            pipelineLayoutName = "stereo1_image";
            
            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
            }

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;
                ds.stencilTestEnable = VK_TRUE;
            
                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_KEEP;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_EQUAL;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0x00;
                stencilOp.reference = 1;
                ds.front = stencilOp;
                ds.back = stencilOp;

                // This is the same as the above flags, but dynamically
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_EQUAL);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0x00);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);            
            
                createPipeline("stereo_image1",
                               pipelineLayoutName,
                               p.fbo->getClearRenderPass(),
                               p.shaders["overlay"],
                               p.vbos["video"],
                               cb, ds);
            }

            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            
            _createBindingSet(p.shaders["overlay"]);
            color = image::Color4f(1.F, 1.F, 1.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               p.shaders["overlay"]->getPushStageFlags(), 0,
                               sizeof(color), &color);
            p.shaders["overlay"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            p.shaders["overlay"]->setFBO("textureSampler",
                                         p.buffers["stereo_image"]);
            _bindDescriptorSets(pipelineLayoutName, "overlay");
            
            // If I draw with colors, the pattern is being drawn.
            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, rgbaMask);
            _vkDraw("video");


            p.fbo->endRenderPass(p.cmd);

            // Draw second image to "stereo_image" buffer

#define SECOND_PASS
#ifdef SECOND_PASS
            if (videoData.size() > 1 && boxes.size() > 1)
            {   
            
                math::Matrix4x4f saved = getTransform();
                const math::Matrix4x4f mvp = saved * math::translate(math::Vector3f(
                                                                   eyeSeparation, 0.F, 0.F));
                setTransform(mvp);
            
                p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
                
                _drawVideo(
                    p.buffers["stereo_image"], "display", 
                    videoData[1], boxes[1],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());  
                
                p.buffers["stereo_image"]->transitionToShaderRead(p.cmd);
                
                setTransform(saved);
            }

            // END FIRST RENDER PASS
            
            // ----- SECOND RENDER PASS OF RIGHT VIDEO
            
            p.fbo->beginLoadRenderPass(p.cmd);


            // Create stencil triangle mesh (scanlines, columns or dots)
            mesh.v.clear();
            mesh.triangles.clear();
            if (stereoType == StereoType::kScanlines)
            {
                mesh = geom::scanlines(1, size);
            }
            else
            {
                mesh = geom::columns(1, size);
            }
            
            p.vbos["stereo"] =
                vlk::VBO::create(mesh.triangles.size() * 3,
                                 vlk::VBOType::Pos2_F32);
            p.vaos["stereo"] = vlk::VAO::create(ctx);
            
            p.vbos["stereo"]->copy(convert(mesh,
                                           p.vbos["stereo"]->getType()));
            

            pipelineLayoutName = "stereo2_stencil";
            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_FALSE;
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;
                ds.stencilTestEnable = VK_TRUE;
            
                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_REPLACE;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0xFF;
                stencilOp.reference = 1;
                ds.front = ds.back = stencilOp;
                
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_REPLACE,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_ALWAYS);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0xFF);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);

                // Draw stencil mask
                createPipeline("stereo2_stencil", pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               p.shaders["wipe"], p.vbos["stereo"],
                               cb, ds);
            }
            
            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            
            _createBindingSet(p.shaders["wipe"]);
            color = image::Color4f(1.F, 0.F, 0.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               p.shaders["wipe"]->getPushStageFlags(), 0,
                               sizeof(color), &color);
            p.shaders["wipe"]->setUniform("transform.mvp", p.transform,
                                          vlk::kShaderVertex);
            _bindDescriptorSets(pipelineLayoutName, "wipe");


            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, rgbaMask);
            _vkDraw("stereo");


            // Draw full RGBA
            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, rgbaMask);
            
            // Draw video
            pipelineLayoutName = "stereo_image2";
            
            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(geom::box(boxes[1], true),
                                              p.vbos["video"]->getType()));
            }

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;
                ds.stencilTestEnable = VK_TRUE;
            
                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_KEEP;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_EQUAL;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0x00;
                stencilOp.reference = 1;
                ds.front = ds.back = stencilOp;
                
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_EQUAL);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0x00);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         1);
                
                createPipeline("stereo_image2",
                               pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               p.shaders["overlay"],
                               p.vbos["video"],
                               cb, ds);
            }
            

            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            
            _createBindingSet(p.shaders["overlay"]);
            color = image::Color4f(1.F, 1.F, 1.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               p.shaders["overlay"]->getPushStageFlags(), 0,
                               sizeof(color), &color);
            p.shaders["overlay"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            p.shaders["overlay"]->setFBO("textureSampler",
                                         p.buffers["stereo_image"]);
            _bindDescriptorSets(pipelineLayoutName, "overlay");

            // If I draw with colors, the pattern is being drawn.
            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, rgbaMask);
            _vkDraw("video");
            
            p.fbo->endRenderPass(p.cmd);
            // END SECOND RENDER PASS
#endif
            
            p.fbo->transitionToShaderRead(p.cmd);
            
            // Transition buffer back to color attachment
            p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
           
        }
        
    } // namespace timeline_vlk
} // namespace tl
