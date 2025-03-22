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

namespace mrv
{

    class NDIView : public Fl_Vk_Window
    {
        void draw() FL_OVERRIDE;

    public:
        NDIView(int x, int y, int w, int h, const char* l = 0);
        NDIView(int w, int h, const char* l = 0);
        ~NDIView();

        void prepare() FL_OVERRIDE;
        void destroy_resources() FL_OVERRIDE;

    protected:
        //! Shaders used in GLFW demo
        VkShaderModule m_vert_shader_module;
        VkShaderModule m_frag_shader_module;

        //! This is for holding a mesh
        Fl_Vk_Mesh m_vertices;
        Fl_Vk_Texture m_textures[DEMO_TEXTURE_COUNT];

        
        void prepare_textures(const uint32_t[1][2]);
        void prepare_vertices();
        void prepare_descriptor_layout();
        void prepare_render_pass();
        void prepare_pipeline();
        void prepare_descriptor_pool();
        void prepare_descriptor_set();

    private:
        void _init();
        void _startThreads();
        void _exitThreads();
        void _findThread();
        void _videoThread();
        void _audioThread();
        void prepare_texture_image(
            const uint32_t* tex_colors, Fl_Vk_Texture* tex_obj,
            VkImageTiling tiling, VkImageUsageFlags usage,
            VkFlags required_props);
        void set_image_layout(
            VkImage image, VkImageAspectFlags aspectMask,
            VkImageLayout old_image_layout, VkImageLayout new_image_layout,
            int srcAccessMaskInt);

        VkShaderModule prepare_vs();
        VkShaderModule prepare_fs();

        TLRENDER_PRIVATE();
    };

} // namespace mrv
