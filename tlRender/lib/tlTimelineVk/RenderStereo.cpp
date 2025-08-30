
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
            

            // Draw left image to "wipe" buffer
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

            float W = 0.F;
            float H = 0.F;
            if (!boxes.empty())
            {
                W = boxes[0].w();
                H = boxes[0].h();
            }

            // Draw stencil triangle mesh
            geom::TriangleMesh2 mesh;
            if (stereoType == StereoType::kScanlines)
            {
                size_t lines = H / 2;
                mesh.v.reserve(lines * 3 * 2);
                mesh.triangles.reserve(lines * 2);
                
                math::Vector2f pts[4];

                size_t idx = 1;
                float y = 0;
                for (float y = 0; y < H; y += 2, idx += 6)
                {
                    pts[0].x = 0;
                    pts[0].y = y;
                    pts[1].x = W;
                    pts[1].y = y;
                    pts[2].x = W;
                    pts[2].y = y + 1;
                    pts[3].x = 0;
                    pts[3].y = y + 1;

                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[1]);
                    mesh.v.push_back(pts[2]);

                    geom::Triangle2 tri;
                    tri.v[0] = idx;
                    tri.v[1] = idx + 1;
                    tri.v[2] = idx + 2;
                    mesh.triangles.push_back(tri);
                    
                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[2]);
                    mesh.v.push_back(pts[3]);
                
                    tri.v[0] = idx + 3;
                    tri.v[1] = idx + 4;
                    tri.v[2] = idx + 5;
                    mesh.triangles.push_back(tri);
                }
            }
            else
            {
                size_t lines = W / 2;
                mesh.v.reserve(lines * 3 * 2);
                mesh.triangles.reserve(lines * 2);
            
                math::Vector2f pts[4];

                size_t idx = 1;
                float x = 0;
                for (float x = 0; x < W; x += 2, idx += 6)
                {
                    pts[0].x = x + 1;
                    pts[0].y = H;

                    pts[1].x = x;
                    pts[1].y = H;

                    pts[2].x = x;
                    pts[2].y = 0;
                    
                    pts[3].x = x + 1;
                    pts[3].y = 0;

                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[1]);
                    mesh.v.push_back(pts[2]);

                    geom::Triangle2 tri;
                    tri.v[0] = idx;
                    tri.v[1] = idx + 1;
                    tri.v[2] = idx + 2;
                    mesh.triangles.push_back(tri);
                    
                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[2]);
                    mesh.v.push_back(pts[3]);
                
                    tri.v[0] = idx + 3;
                    tri.v[1] = idx + 4;
                    tri.v[2] = idx + 5;
                    mesh.triangles.push_back(tri);
                }
            }
            
            p.vbos["stereo"] =
                vlk::VBO::create(mesh.triangles.size() * 3,
                                 vlk::VBOType::Pos2_F32);
            p.vaos["stereo"] = vlk::VAO::create(ctx);
            
            p.vbos["stereo"]->copy(convert(mesh,
                                           p.vbos["stereo"]->getType()));

            // ----- FIRST RENDER PASS OF LEFT VIDEO
            p.fbo->beginClearRenderPass(p.cmd);
            
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
            
            _vkDraw("video");
            
            p.fbo->endRenderPass(p.cmd);
            // END FIRST RENDER PASS

            // ----- SECOND RENDER PASS OF RIGHT VIDEO

            // Draw second image to "stereo_image" buffer
            
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

            //
            //  DRAW STEREO STENCIL MASK
            //
            
            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);
            
            p.fbo->beginLoadRenderPass(p.cmd);
                        

            
            pipelineLayoutName = "stereo1_stencil";
            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_FALSE;
                // colorBlendAttachment.colorWriteMask = 0;
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

                ds.front = stencilOp;
                ds.back = stencilOp;

            
                // Draw stencil mask
                createPipeline("stereo1_stencil", pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               p.shaders["wipe"], p.vbos["wipe"],
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

            _vkDraw("stereo");

            p.fbo->endRenderPass(p.cmd);
            
#if 0
            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);
            
            p.fbo->beginLoadRenderPass(p.cmd);
            
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
                stencilOp.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0x00;
                stencilOp.reference = 1;

                ds.front = stencilOp;
                ds.back = stencilOp;

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
