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

#include <tlCore/Util.h>

#include <FL/Fl_Vk_Window.H>

#define DEMO_TEXTURE_COUNT 1

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

    class NDIView : public Fl_Vk_Window
    {
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
        Fl_Vk_Texture m_textures[DEMO_TEXTURE_COUNT];

        void init_vk_swapchain() FL_OVERRIDE;

        void prepare_textures();
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
        void prepare_texture_image(
            const float* tex_colors, Fl_Vk_Texture* tex_obj,
            VkImageTiling tiling, VkImageUsageFlags usage,
            VkFlags required_props);

        VkShaderModule prepare_vs();
        VkShaderModule prepare_fs();

        void _create_HDR_shader();
        void start();
        
        TLRENDER_PRIVATE();
    };

} // namespace mrv
