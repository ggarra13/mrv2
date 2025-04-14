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

#pragma once

#include <memory>

#include <half.h>

#include <tlCore/Util.h>
#include <tlCore/ListObserver.h>

class Fl_Menu_;
#include <FL/Fl_Vk_Window.H>

struct pl_shader_res;

namespace tl
{
    namespace image
    {
        class Image;
    }
} // namespace tl

namespace mrv
{
    using namespace tl;

    class NDIView : public Fl_Vk_Window
    {
        void vk_draw_begin() FL_OVERRIDE;
        void draw() FL_OVERRIDE;

    public:
        NDIView(int x, int y, int w, int h, const char* l = 0);
        NDIView(int w, int h, const char* l = 0);
        virtual ~NDIView();

        //! Observe the NDI sources
        std::shared_ptr<observer::IList<std::string> >
        observeNDISources() const;

        //! Set a new NDI Source
        void setNDISource(const std::string&);

        
        void prepare() FL_OVERRIDE;
        void destroy_resources() FL_OVERRIDE;
        
        std::vector<const char*> get_required_extensions() FL_OVERRIDE;
        std::vector<const char*> get_optional_extensions() FL_OVERRIDE;
        std::vector<const char*> get_device_extensions() FL_OVERRIDE;

    protected:
        //! Shaders used in GLFW demo
        VkShaderModule m_vert_shader_module;
        VkShaderModule m_frag_shader_module;

        //! This is for holding a mesh
        Fl_Vk_Mesh m_vertices;
        std::vector<Fl_Vk_Texture> m_textures;

        void init_vk_swapchain() FL_OVERRIDE;

        void prepare_main_texture();
        void prepare_shader();
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

        void fill_menu_bar(Fl_Menu_*);

        VkShaderModule prepare_vs();
        VkShaderModule prepare_fs();

        void destroy_textures();

        void uploadTextureData(
            VkImage image, uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, const int channels,
            const int pix_fmt_size, const void* data);

        void addGPUTextures(const pl_shader_res*);

    protected:
        TLRENDER_PRIVATE();
    };

} // namespace mrv
