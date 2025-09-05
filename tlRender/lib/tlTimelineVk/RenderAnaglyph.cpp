
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

        void Render::drawAnaglyph(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const float eyeSeparation,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            //
            // Some constants
            // 
            const VkColorComponentFlags rgbaMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
            const VkColorComponentFlags redMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT };
            const VkColorComponentFlags cyanMask[] =
                { VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                  VK_COLOR_COMPONENT_A_BIT };
            

            //
            // Some auxiliary variables
            //
            image::Color4f color(1.F, 0.F, 0.F);
            VkPipelineLayout pipelineLayout;
            std::string pipelineLayoutName = "stereo1";

            //
            // Create offscreen buffer to draw into
            // 
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

// \@bug: If I set LEFT_IMAGE to 1 only, I get a red video
//        If I set RIGHT_IMAGE to 1 only, I get a cyan video
//        If I set LEFT_IMAGE AND RIGHT_IMAGE, I get the left image rendered normally.
#define LEFT_IMAGE 1
#define RIGHT_IMAGE 1
            
#if LEFT_IMAGE

            // Draw left image to "stereo_image" buffer
            if (!videoData.empty() && !boxes.empty())
            {
#if USE_DYNAMIC_RGBA_WRITE_MASKS
                ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, redMask);
#endif
                
                p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
                
                _drawVideo(
                    p.buffers["stereo_image"], "display", 
                    videoData[0], boxes[0],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                
                p.buffers["stereo_image"]->transitionToShaderRead(p.cmd);
            }

            // ----- FIRST RENDER PASS OF LEFT EYE VIDEO
            
            // Main FBO Transition
            p.fbo->transitionToColorAttachment(p.cmd);
            
            p.fbo->beginClearRenderPass(p.cmd);


            // Draw video
            pipelineLayoutName = "stereo_image1";
            
            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
            }

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
#if ! USE_DYNAMIC_RGBA_WRITE_MASKS
                colorBlendAttachment.colorWriteMask = *redMask;
#endif
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;
                ds.stencilTestEnable = VK_FALSE;
                
                createPipeline("stereo_image1",
                               pipelineLayoutName,
                               p.fbo->getClearRenderPass(),
                               p.shaders["overlay"],
                               p.vbos["video"],
                               cb, ds);
            }

            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

            //
            // Prepare shaders
            //
            _createBindingSet(p.shaders["overlay"]);
            color = image::Color4f(1.F, 1.F, 1.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               p.shaders["overlay"]->getPushStageFlags(), 0,
                               sizeof(color), &color);
            p.shaders["overlay"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            p.shaders["overlay"]->setFBO("textureSampler",
                                         p.buffers["stereo_image"]);
            _bindDescriptorSets(pipelineLayoutName, "overlay");

            //
            // Draw with RGBA the video
            //
            _vkDraw("video");

            p.fbo->endRenderPass(p.cmd);
#endif
            // END FIRST RENDER PASS            
            
            // ----- SECOND RENDER PASS OF RIGHT VIDEO            

#if RIGHT_IMAGE
            // Draw second image to "stereo_image" buffer
            if (videoData.size() > 1 && boxes.size() > 1)
            {   
                math::Matrix4x4f saved = getTransform();
                const math::Matrix4x4f mvp = saved * math::translate(math::Vector3f(
                                                                   eyeSeparation, 0.F, 0.F));
                setTransform(mvp);
            
#if USE_DYNAMIC_RGBA_WRITE_MASKS
                ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, cyanMask);
#endif
                p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
                
                _drawVideo(
                    p.buffers["stereo_image"], "display", 
                    videoData[1], boxes[1],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());  
                
                p.buffers["stereo_image"]->transitionToShaderRead(p.cmd);
                
                setTransform(saved);
            }

            // Main FBO Transitions
            p.fbo->transitionToColorAttachment(p.cmd);
            
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
#if ! USE_DYNAMIC_RGBA_WRITE_MASKS
                colorBlendAttachment.colorWriteMask = *cyanMask;
#endif
                cb.attachments.push_back(colorBlendAttachment);
            
                vlk::DepthStencilStateInfo ds;
                
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
                        
#if USE_DYNAMIC_RGBA_WRITE_MASKS
            ctx.vkCmdSetColorWriteMaskEXT(p.cmd, 0, 1, rgbaMask);
#endif
            
            p.fbo->transitionToShaderRead(p.cmd);
            
            // Transition buffer back to color attachment
            p.buffers["stereo_image"]->transitionToColorAttachment(p.cmd);
           
        }       
    }
}
