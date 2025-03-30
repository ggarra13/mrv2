//
// Tiny Vulkan demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <memory>

#include <half.h>

#include <tlCore/Util.h>

#include <FL/Fl_Vk_Window.H>

struct pl_shader_res;

namespace tl
{
    namespace image
    {
        class Image;
    }
}


namespace mrv
{
    using namespace tl;

    struct TextureUploadResources {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkFence fence; // To track completion
    };
    
    class NDIView : public Fl_Vk_Window
    {
        void vk_draw_begin() FL_OVERRIDE;
        void draw() FL_OVERRIDE;

    public:
        NDIView(int x, int y, int w, int h, const char* l = 0);
        NDIView(int w, int h, const char* l = 0);
        virtual ~NDIView();

        void prepare() FL_OVERRIDE;
        void destroy_resources() FL_OVERRIDE;

    protected:
        //! Shaders used in GLFW demo
        VkShaderModule m_vert_shader_module;
        VkShaderModule m_frag_shader_module;

        //! This is for holding a mesh
        Fl_Vk_Mesh m_vertices;
        std::vector<Fl_Vk_Texture> m_textures;

        void init_vk_swapchain() FL_OVERRIDE;

        void prepare_main_texture();
        void prepare_vertices();
        void prepare_descriptor_layout();
        void prepare_render_pass();
        void prepare_pipeline();
        void prepare_descriptor_pool();
        void prepare_descriptor_set();

        void update_texture();
        
    private:
        void _init();
        void _copy(const uint8_t* video_frame);
        void _startThreads();
        void _exitThreads();
        void _findThread();
        void _videoThread();
        void _audioThread();
        void prepare_texture_image(Fl_Vk_Texture* tex_obj,
                                   VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkFlags required_props);

        VkShaderModule prepare_vs();
        VkShaderModule prepare_fs();

        void create_HDR_shader();
        void start();
        void destroy_textures();

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);


        void cleanupCompletedTransitions();
        void cleanupCompletedUploads();
        
        void transitionImageLayout(VkImage image,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout);
        
        VkImage createImage(
            VkImageType imageType,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            VkFormat format,
            VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
            VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT
            );
        
        VkDeviceMemory allocateAndBindImageMemory(VkImage image);
        void createBuffer(
             VkDeviceSize size,
             VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties,
             VkBuffer& buffer,
             VkDeviceMemory& bufferMemory);
        
        void uploadTextureData(VkImage image,
                                uint32_t width,
                                uint32_t height,
                                uint32_t depth,
                                VkFormat format,
                                const void* data);
        
        VkImageView createImageView(VkImage image, VkFormat format, VkImageType imageType);
        VkSampler createSampler();
        
        void addGPUTextures(const pl_shader_res*);
        
    protected:
        TLRENDER_PRIVATE();
    };

} // namespace mrv
