// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#define LOG_WARNING(x) std::cerr << x << std::endl;

#ifdef NDEBUG
#  define LOG_DEBUG(x)
#else
#  define LOG_DEBUG(x) std::cerr << x << std::endl;
#endif

#include <Imath/half.h>

#include <tlCore/Image.h>
#include <tlCore/HDR.h>
#include <tlCore/StringFormat.h>

#include <tlVk/Mesh.h>
#include <tlVk/PipelineCreationState.h>
#include <tlVk/Shader.h>

#include <rapidxml/rapidxml.hpp>

#include <tlDevice/NDI/NDI.h>

#include "mrvHDR/mrvDesktop.h"
#include "mrvHDR/mrvNDICallbacks.h"
#include "mrvHDR/mrvMonitor.h"

#include "mrvCore/mrvI8N.h"

#include "hdr/mrvHDRApp.h"
#include "mrvNDIView.h"

extern "C"
{
#include <libplacebo/dummy.h>
#include <libplacebo/shaders/colorspace.h>
#include <libplacebo/shaders.h>
}

#include "mrvHDRView.h"

// Must come last due to X11 macros
#include <FL/platform.H>
#include <FL/vk.h>
#include <FL/Fl.H>
#include <FL/vk_enum_string_helper.h>
#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Button.H>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <regex>
#include <thread>

#define LOG_STATUS(x) std::cout << x << std::endl;


namespace
{
    const char* kModule = "ndi_viewer";
} // namespace

namespace
{
    float apply_inverse_pq(float x)
    {
        float m1 = 0.8359375f;
        float m2 = 18.8515625f;
        float c1 = 0.8359375f;
        float c2 = 18.6875f;
        float c3 = 2.402f;

        if (x <= c1)
        {
            return x / m1;
        }
        else
        {
            return pow((x + c2) / c3, 1.0f / m2);
        }
    }

    // Function to unescape &quot; back to normal quotes (")
    inline std::string
    unescape_quotes_from_xml(const std::string& xml_escaped_str)
    {
        std::string json_str;
        size_t pos = 0;
        while (pos < xml_escaped_str.size())
        {
            if (xml_escaped_str.compare(pos, 6, "&quot;") == 0)
            {
                json_str += '"'; // Replace &quot; with "
                pos += 6;        // Skip over &quot;
            }
            else
            {
                json_str += xml_escaped_str[pos];
                pos++;
            }
        }
        return json_str;
    }

    VkFormat to_vk_format(pl_fmt fmt)
    {
        int size = fmt->internal_size / fmt->num_components;

        switch (fmt->num_components)
        {
        case 1:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16_SFLOAT : VK_FORMAT_R32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32_UINT
                    : (size == 2 ? VK_FORMAT_R16_UINT
                       : VK_FORMAT_R8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32_SINT
                    : (size == 2 ? VK_FORMAT_R16_SINT
                       : VK_FORMAT_R8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16_UNORM : VK_FORMAT_R8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16_SNORM : VK_FORMAT_R8_SNORM;
            }
            break;
        case 2:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16_SFLOAT
                    : VK_FORMAT_R32G32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_UINT
                    : (size == 2 ? VK_FORMAT_R16G16_UINT
                       : VK_FORMAT_R8G8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_SINT
                    : (size == 2 ? VK_FORMAT_R16G16_SINT
                       : VK_FORMAT_R8G8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_UNORM
                    : VK_FORMAT_R8G8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_SNORM
                    : VK_FORMAT_R8G8_SNORM;
            }
            break;
        case 3:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SFLOAT
                    : VK_FORMAT_R32G32B32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_UINT
                    : (size == 2 ? VK_FORMAT_R16G16B16_UINT
                       : VK_FORMAT_R8G8B8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_SINT
                    : (size == 2 ? VK_FORMAT_R16G16B16_SINT
                       : VK_FORMAT_R8G8B8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_UNORM
                    : VK_FORMAT_R8G8B8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SNORM
                    : VK_FORMAT_R8G8B8_SNORM;
            }
            break;
        case 4:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SFLOAT
                    : VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_UINT
                    : (size == 2 ? VK_FORMAT_R16G16B16A16_UINT
                       : VK_FORMAT_R8G8B8A8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_SINT
                    : (size == 2 ? VK_FORMAT_R16G16B16A16_SINT
                       : VK_FORMAT_R8G8B8A8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_UNORM
                    : VK_FORMAT_R8G8B8A8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SNORM
                    : VK_FORMAT_R8G8B8A8_SNORM;
            }
            break;
        }

        return VK_FORMAT_UNDEFINED;
    }

    std::string vertexSource()
    {
        return R"(
 #version 450
 layout(location = 0) in vec2 inPos;
 layout(location = 1) in vec2 inTexCoord;
 layout(location = 0) out vec2 outTexCoord;

 void main() {
      gl_Position = vec4(inPos, 0.0, 1.0);
      outTexCoord = inTexCoord;
 }
 )";
    }
} // namespace

namespace mrv
{
    
    monitor::HDRCapabilities getHDRCapabilities(int screen_num)
    {
        monitor::HDRCapabilities out;
        if (desktop::Wayland())
        {
            const std::string& monitorName = desktop::monitorName(screen_num);
            const auto names = string::split(monitorName, ':');
            std::string connector;
            if (!names.empty())
            {
                connector = names[0];
                out = monitor::get_hdr_capabilities_by_name(connector);
            }
            else
            {
                LOG_WARNING("Could not determine monitor connector.  Using first connector on list.");
                // Last resort, use any hdr monitor if present
                out = monitor::get_hdr_capabilities(-1);
            }
        }
        else
        {
            out = monitor::get_hdr_capabilities(screen_num);
        }
        return out;
    }

    void NDIView::addGPUTextures(const pl_shader_res* res)
    {
        for (unsigned i = 0; i < res->num_descriptors; ++i)
        {
            const pl_shader_desc* sd = &res->descriptors[i];
            switch (sd->desc.type)
            {
            case PL_DESC_STORAGE_IMG:
                throw std::runtime_error("Unimplemented storage image");
            case PL_DESC_SAMPLED_TEX:
            {
                pl_tex tex = reinterpret_cast<pl_tex>(sd->binding.object);
                pl_fmt fmt = tex->params.format;

                int dims = pl_tex_params_dimension(tex->params);
                assert(dims >= 1 && dims <= 3);

                const char* samplerName = sd->desc.name;
                int pixel_fmt_size = fmt->internal_size / fmt->num_components;
                int channels = fmt->num_components;

                // Map libplacebo format to Vulkan format
                VkFormat imageFormat = to_vk_format(fmt);

                // Texture dimensions
                uint32_t width = tex->params.w;
                uint32_t height = (dims >= 2) ? tex->params.h : 1;
                uint32_t depth = (dims == 3) ? tex->params.d : 1;

                assert(width > 0);
                if (dims >= 2)
                    assert(height > 0);
                if (dims == 3)
                    assert(depth > 0);

                // Get texture data from libplacebo
                const void* values = pl_tex_dummy_data(tex);
                if (!values)
                {
                    throw std::runtime_error(
                        "Could not read pl_tex_dummy_data");
                }

                // Determine image type
                VkImageType imageType = (dims == 1)   ? VK_IMAGE_TYPE_1D
                                        : (dims == 2) ? VK_IMAGE_TYPE_2D
                                        : VK_IMAGE_TYPE_3D;

                auto texture = vlk::Texture::create(ctx, imageType, width,
                                                    height, depth, imageFormat,
                                                    samplerName);
                texture->copy(reinterpret_cast<const uint8_t*>(values),
                              width * height * depth * pixel_fmt_size * channels);
                m_textures.push_back(texture);
                break;
            }
            default:
                throw std::runtime_error("Unknown texture type");
            }
        }
    }

    struct LibPlaceboData
    {
        LibPlaceboData();
        ~LibPlaceboData();

        pl_log log;
        pl_gpu gpu;
        pl_shader shader = nullptr;
        pl_shader_obj state = nullptr;
        const pl_shader_res* res = nullptr;
        
        std::vector<pl_shader_var> pcUBOvars;
        void* pcUBOData = nullptr;
        size_t pcUBOSize = 0;
    };

    LibPlaceboData::LibPlaceboData()
    {
        log = pl_log_create(PL_API_VER, NULL);
        if (!log)
        {
            throw std::runtime_error("log creation failed");
        }

        struct pl_gpu_dummy_params gpu_dummy;
        memset(&gpu_dummy, 0, sizeof(pl_gpu_dummy_params));

        gpu_dummy.glsl.version = 450;
        gpu_dummy.glsl.gles = false;
        gpu_dummy.glsl.vulkan = false;
        gpu_dummy.glsl.compute = false;

        gpu_dummy.limits.callbacks = false;
        gpu_dummy.limits.thread_safe = true;
        /* pl_buf */
        gpu_dummy.limits.max_buf_size = SIZE_MAX;
        gpu_dummy.limits.max_ubo_size = SIZE_MAX;
        gpu_dummy.limits.max_ssbo_size = SIZE_MAX;
        gpu_dummy.limits.max_vbo_size = SIZE_MAX;
        gpu_dummy.limits.max_mapped_size = SIZE_MAX;
        gpu_dummy.limits.max_buffer_texels = UINT64_MAX;
        /* pl_tex */
        gpu_dummy.limits.max_tex_1d_dim = UINT32_MAX;
        gpu_dummy.limits.max_tex_2d_dim = UINT32_MAX;
        gpu_dummy.limits.max_tex_3d_dim = UINT32_MAX;
        gpu_dummy.limits.buf_transfer = true;
        gpu_dummy.limits.align_tex_xfer_pitch = 1;
        gpu_dummy.limits.align_tex_xfer_offset = 1;

        /* pl_pass */
        gpu_dummy.limits.max_variable_comps = SIZE_MAX;
        gpu_dummy.limits.max_constants = SIZE_MAX;
        gpu_dummy.limits.max_pushc_size = SIZE_MAX;
        gpu_dummy.limits.max_dispatch[0] = UINT32_MAX;
        gpu_dummy.limits.max_dispatch[1] = UINT32_MAX;
        gpu_dummy.limits.max_dispatch[2] = UINT32_MAX;
        gpu_dummy.limits.fragment_queues = 0;
        gpu_dummy.limits.compute_queues = 0;

        gpu = pl_gpu_dummy_create(log, &gpu_dummy);
        if (!gpu)
        {
            throw std::runtime_error("pl_gpu_dummy_create failed!");
        }
        
        pl_shader_params shader_params;
        memset(&shader_params, 0, sizeof(pl_shader_params));

        shader_params.id = 1;
        shader_params.gpu = gpu;
        shader_params.dynamic_constants = false;

        shader = pl_shader_alloc(log, &shader_params);
        if (!shader)
        {
            throw std::runtime_error("pl_shader_alloc failed!");
        }
    }

    LibPlaceboData::~LibPlaceboData()
    {
        pl_shader_free(&shader);
        shader = nullptr;
        res = nullptr;
        if (state)
        {
            pl_shader_obj_destroy(&state);
            state = nullptr;
        }
        pcUBOvars.clear();
        free(pcUBOData);
        pcUBOSize = 0;
            
        pl_gpu_dummy_destroy(&gpu);
        pl_log_destroy(&log);
    }
    
    struct NDIView::Private
    {
        // FLTK state variables
        bool useHDRMetadata = false;

        // HDR monitor variables
        int screen_index = 0;
        monitor::HDRCapabilities hdrCapabilities;
        const pl_shader_res* res = nullptr;
        pl_shader_obj state = nullptr;

        NDIlib_find_instance_t NDI_find = nullptr;
        NDIlib_recv_instance_t NDI_recv = nullptr;

        std::shared_ptr<observer::List<std::string> > NDISources;
        std::shared_ptr<observer::ListObserver<std::string> >
        NDISourcesObserver;
        std::string currentNDISource;

        bool init = false;

        struct FindMutex
        {
            std::mutex mutex;
        };
        FindMutex findMutex;
        struct FindThread
        {
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        FindThread findThread;

        struct VideoMutex
        {
            std::mutex mutex;
        };
        VideoMutex videoMutex;
        struct VideoThread
        {
            std::chrono::steady_clock::time_point logTimer;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        VideoThread videoThread;

        struct AudioMutex
        {
            std::mutex mutex;
        };
        AudioMutex audioMutex;
        struct AudioThread
        {
            std::chrono::steady_clock::time_point logTimer;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        AudioThread audioThread;

        // Standard NDI attributes
        std::string primariesName;
        std::string transferName;
        std::string matrixName;

        // Full mrv2 image data (we try to use this)
        bool hdrMonitorFound = false;
        bool isP3Display = false;

        bool hasHDR = false;
        image::HDRData hdrData;

        std::string oldShaderSource;
        unsigned bindingIndex = 1;
        std::shared_ptr<vlk::Shader> shader;

        NDIlib_FourCC_video_type_e fourCC = NDIlib_FourCC_type_UYVY;

        // Vulkan variables
        VkColorSpaceKHR lastColorSpace;
        vlk::PipelineCreationState pipelineState;

        // tlRender variables
        image::Info info;
        std::shared_ptr<image::Image> image;

        // LibPlacebo variables
        std::shared_ptr<LibPlaceboData> placeboData;
        std::string hdrColors;
        std::string hdrColorsDef;

        // Rendering variables
        std::shared_ptr<tl::vlk::VBO> vbo;
        std::shared_ptr<tl::vlk::VAO> vao;
    };


    NDIView::~NDIView()
    {
        TLRENDER_P();

        _exitThreads();

        p.findThread.running = false;
        if (p.findThread.thread.joinable())
            p.findThread.thread.join();
    }

    NDIView::NDIView(int x, int y, int w, int h, const char* l) :
        Fl_Vk_Window(x, y, w, h, l),
        _p(new Private)
    {
        _init();
    }

    NDIView::NDIView(int w, int h, const char* l) :
        Fl_Vk_Window(w, h, l),
        _p(new Private)
    {
        _init();
    }

    void NDIView::init_colorspace()
    {
        TLRENDER_P();
        
        Fl_Vk_Window::init_colorspace();

        // Look for HDR10 or HLG if present
        p.hdrMonitorFound = false;

        bool valid_colorspace = false;
        switch (colorSpace())
        {
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
            valid_colorspace = true;
            break;
        default:
            break;
        }

        p.screen_index = this->screen_num();
        p.hdrCapabilities = getHDRCapabilities(p.screen_index);
        
        if (valid_colorspace && p.hdrCapabilities.supported)
        {
            p.hdrMonitorFound = true;
            LOG_STATUS(_("HDR monitor found."));
            
            _getMonitorNits(false);
        }
        else
        {
            colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            format() = VK_FORMAT_B8G8R8A8_UNORM;
            LOG_STATUS(_("HDR monitor not found or not configured."));
        }
        LOG_STATUS("Vulkan color space is " << string_VkColorSpaceKHR(colorSpace()));
        LOG_STATUS("Vulkan format is " << string_VkFormat(format()));

        p.lastColorSpace = colorSpace();
    }

    void NDIView::_copy(const uint8_t* video_frame, int stride_in_bytes)
    {
        TLRENDER_P();

        const auto& info = p.image->getInfo();
        const std::size_t w = p.info.size.w;
        const std::size_t h = p.info.size.h;
        uint8_t* const data = p.image->getData();
        half* rgba = (half*)data;

        switch (p.fourCC)
        {
        case NDIlib_FourCC_type_PA16:
        {
            const int stride_words = stride_in_bytes / 2;
            uint16_t* p_y = (uint16_t*)video_frame;
            const uint16_t* p_uv = p_y + h * stride_words;
            const uint16_t* p_alpha = p_uv + h * stride_words;

            // Determine BT.601 or BT.709 based on resolution
            bool useBT709 = (w >= 1280 && h >= 720);

            // Coefficients
            float Kr = useBT709 ? 0.2126f : 0.299f;
            float Kb = useBT709 ? 0.0722f : 0.114f;
            float Kg = 1.0f - Kr - Kb;

            for (int y = 0; y < h; ++y)
            {
                // Calculate base pointers for the current row
                const uint16_t* row_y = p_y + (y * stride_words);
                const uint16_t* row_uv = p_uv + (y * stride_words);
                const uint16_t* row_alpha = p_alpha + (y * stride_words);
        
                for (int x = 0; x < w; ++x)
                {
                    // Extract Y and Alpha
                    float Yf = row_y[x] / 65535.0f;
                    float A = row_alpha[x] / 65535.0f;

                    // Extract U and V (4:2:2 interleaved means 1 UV pair per 2 pixels)
                    int uv_idx = (x / 2) * 2;
                    float Uf = (row_uv[uv_idx] - 32768) / 65535.0f;
                    float Vf = (row_uv[uv_idx + 1] - 32768) / 65535.0f;

                    float R, G, B;
                    float Y_linear = Yf;

                    if (useBT709)
                    {
                        R = Y_linear + 1.5748f * Vf;
                        G = Y_linear - 0.1873f * Uf - 0.4681f * Vf;
                        B = Y_linear + 1.8556f * Uf;
                    }
                    else
                    {
                        R = Y_linear + 1.402f * Vf;
                        G = Y_linear - 0.344f * Uf - 0.714f * Vf;
                        B = Y_linear + 1.772f * Uf;
                    }

                    // Store as RGBA float
                    int rgba_index = (y * w + x) * 4; 
                    rgba[rgba_index] = R;
                    rgba[rgba_index + 1] = G;
                    rgba[rgba_index + 2] = B;
                    rgba[rgba_index + 3] = A;
                }
            }
            break;
        }
        case NDIlib_FourCC_type_P216:
        {
            std::cerr << "P216" << std::endl;
            const int stride_words = stride_in_bytes / 2;
            uint16_t* p_y = (uint16_t*)video_frame;
            const uint16_t* p_uv = p_y + h * stride_words;            
            
            // Determine BT.601 or BT.709 based on resolution
            bool useBT709 = (w >= 1280 && h >= 720);

            // Coefficients
            float Kr = useBT709 ? 0.2126f : 0.299f;
            float Kb = useBT709 ? 0.0722f : 0.114f;
            float Kg = 1.0f - Kr - Kb;

            for (int y = 0; y < h; ++y)
            {
                // Calculate base pointers for the current row
                const uint16_t* row_y = p_y + (y * stride_words);
                const uint16_t* row_uv = p_uv + (y * stride_words);
        
                for (int x = 0; x < w; ++x)
                {
                    // Extract Y and Alpha
                    float Yf = row_y[x] / 65535.0f;
                    
                    // Extract U and V (4:2:2 interleaved means 1 UV pair per 2 pixels)
                    int uv_idx = (x / 2) * 2;
                    float Uf = (row_uv[uv_idx] - 32768) / 65535.0f;
                    float Vf = (row_uv[uv_idx + 1] - 32768) / 65535.0f;
                    
                    float R, G, B;

                    float Y_linear = Yf;

                    // apply_inverse_pq is not needed
                    // if (p.hasHDR)
                    //     Y_linear = apply_inverse_pq(Yf);
                    // else
                    //     Y_linear = Yf;

                    if (useBT709)
                    {
                        // BT.709 typical multipliers
                        // (Exact values differ slightly in various references)
                        R = Y_linear + 1.5748f * Vf;
                        G = Y_linear - 0.1873f * Uf - 0.4681f * Vf;
                        B = Y_linear + 1.8556f * Uf;
                    }
                    else
                    {
                        // BT.601
                        R = Y_linear + 1.402f * Vf;
                        G = Y_linear - 0.344f * Uf - 0.714f * Vf;
                        B = Y_linear + 1.772f * Uf;
                    }

                    // Store as RGBA float
                    int rgba_index = (y * w + x) * 4;
                    rgba[rgba_index] = R;
                    rgba[rgba_index + 1] = G;
                    rgba[rgba_index + 2] = B;
                    rgba[rgba_index + 3] = 1.0F;
                }
            }
            break;
        }
        case NDIlib_FourCC_type_UYVY:
        {
            std::cerr << "UYVY" << std::endl;
            break;
        }
        case NDIlib_FourCC_type_UYVA:
        {
            std::cerr << "UYVA" << std::endl;
            break;
        }
        default:
            std::cerr << "Unknown format" << std::endl;
            break;
        }
    }

    void NDIView::prepare_main_texture()
    {
        TLRENDER_P();

        VkResult result;
        m_textures.reserve(16);  // reserve up to 16 textures for libplacebo.
        
        uint32_t tex_width = 1, tex_height = 1;
        image::Info info;
        if (p.image)
        {
            info = p.image->getInfo();
        }
        else
        {
            info = image::Info(1, 1, image::PixelType::RGBA_F16);
        }
        m_textures.push_back(vlk::Texture::create(ctx, info));
        
        // Transition image layout to TRANSFER_SHADER_READ_OPTIMAL
        VkCommandBuffer cmd = beginSingleTimeCommands(device(), commandPool());
        m_textures[0]->transitionToShaderRead(cmd);
        endSingleTimeCommands(cmd, device(), commandPool(), queue());
    }

    void NDIView::prepare_vertices()
    {
        TLRENDER_P();

        using namespace tl;

        const math::Size2i viewportSize = {pixel_w(), pixel_h()};
        image::Size renderSize = {pixel_w(), pixel_h()};

        if (p.image)
        {
            renderSize = p.image->getSize();
            renderSize.w *= renderSize.pixelAspectRatio;
        }

        float aspectRender = renderSize.w / static_cast<float>(renderSize.h);
        float aspectViewport =
            viewportSize.w / static_cast<float>(viewportSize.h);

        float scaleX = 1.0F;
        float scaleY = 1.0F;

        if (aspectRender < aspectViewport)
        {
            // Image is too wide, shrink X
            scaleX = aspectRender / aspectViewport;
        }
        else
        {
            // Image is too tall, shrink Y
            scaleY = aspectViewport / aspectRender;
        }

        const geom::TriangleMesh2& mesh =
            geom::box(math::Box2f(-scaleX, -scaleY, scaleX * 2, scaleY * 2));

        const size_t numTriangles = mesh.triangles.size();

        if (!p.vbo || (p.vbo && p.vbo->getSize() != numTriangles * 3))
        {
            p.vbo = vlk::VBO::create(
                numTriangles * 3, vlk::VBOType::Pos2_F32_UV_U16);
            p.vao.reset();
        }
        if (p.vbo)
        {
            p.vbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
        }

        if (!p.vao && p.vbo)
        {
            p.vao = vlk::VAO::create(ctx);
            p.vao->upload(p.vbo->getData());
        }
    }

    // creates m_renderPass
    void NDIView::prepare_render_pass()
    {
        if (m_renderPass != VK_NULL_HANDLE)
            return;
        
        VkAttachmentDescription attachments[2];
        attachments[0] = VkAttachmentDescription();
        attachments[0].format = ctx.format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout =
            VK_IMAGE_LAYOUT_UNDEFINED; // Start undefined
        attachments[0].finalLayout =
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

        attachments[1] = VkAttachmentDescription();

        VkAttachmentReference color_reference = {};
        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = NULL;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = NULL;

        VkRenderPassCreateInfo rp_info = {};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rp_info.pNext = NULL;
        rp_info.attachmentCount = 1;
        rp_info.pAttachments = attachments;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = NULL;

        VkResult result;
        result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
        VK_CHECK(result);
    }


    void NDIView::prepare_pipeline()
    {
        TLRENDER_P();

        VkGraphicsPipelineCreateInfo pipeline = {};
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};

        vlk::VertexInputStateInfo vi;
        vi.bindingDescriptions = p.vbo->getBindingDescription();
        vi.attributeDescriptions = p.vbo->getAttributes();
            
        // Defaults are fine
        vlk::InputAssemblyStateInfo ia;
        
        // Defaults are fine
        vlk::RasterizationStateInfo rs;
        
        // Defaults are fine
        vlk::ViewportStateInfo vp;
        
        // Defaults are fine
        vlk::DynamicStateInfo dynamicState;
        dynamicState.dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
        colorBlendAttachment.blendEnable = VK_FALSE;
        
        vlk::ColorBlendStateInfo cb;
        cb.attachments.push_back(colorBlendAttachment);
        
        vlk::DepthStencilStateInfo ds;
        ds.depthTestEnable = mode() & FL_DEPTH ? VK_TRUE : VK_FALSE;
        ds.depthWriteEnable = mode() & FL_DEPTH ? VK_TRUE : VK_FALSE;
        ds.stencilTestEnable = mode() & FL_STENCIL ? VK_TRUE : VK_FALSE;

        vlk::MultisampleStateInfo ms;
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkResult result;

        // Get the vertex and fragment shaders
        std::vector<vlk::PipelineCreationState::ShaderStageInfo>
            shaderStages(2);
        
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].name = p.shader->getName();
        shaderStages[0].module = p.shader->getVertex();
        shaderStages[0].entryPoint = "main";

        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[0].name = p.shader->getName();
        shaderStages[1].module = p.shader->getFragment();
        shaderStages[1].entryPoint = "main";

        vlk::PipelineCreationState pipelineState;
        pipelineState.vertexInputState = vi;
        pipelineState.inputAssemblyState = ia;
        pipelineState.colorBlendState = cb;
        pipelineState.rasterizationState = rs;
        pipelineState.depthStencilState = ds;
        pipelineState.viewportState = vp;
        pipelineState.multisampleState = ms;
        pipelineState.dynamicState = dynamicState;
        pipelineState.stages = shaderStages;
        pipelineState.renderPass = m_renderPass;
        pipelineState.layout = m_pipeline_layout;

        if (pipelineState != p.pipelineState)
        {
            VkDevice device = ctx.device;

            if (m_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device, m_pipeline, nullptr);
            }
            
            m_pipeline = pipelineState.create(device);
            p.pipelineState = pipelineState;
        }
    }

    void NDIView::prepare_descriptor_layout()
    {
        TLRENDER_P();

        if (m_desc_layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device(), m_desc_layout, nullptr);
            m_desc_layout = VK_NULL_HANDLE;
        }
        
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        // Main texture at binding 0
        VkDescriptorSetLayoutBinding mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mainBinding.descriptorCount = 1;
        mainBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        mainBinding.pImmutableSamplers = nullptr;
        bindings.push_back(mainBinding);

        // Additional libplacebo textures starting at binding 1
        for (uint32_t i = 1; i < m_textures.size(); ++i)
        {
            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = i; // Matches libplacebo’s binding = i + 1 in GLSL
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.descriptorCount = 1;
            binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(
            device(), &layoutInfo, nullptr, &m_desc_layout);
        VK_CHECK(result);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_desc_layout;

        VkPushConstantRange pushConstantRange = {};
        std::size_t pushSize = p.shader->getPushSize();
        if (pushSize > 0)
        {
            pushConstantRange.stageFlags = p.shader->getPushStageFlags();
            pushConstantRange.offset = 0;
            pushConstantRange.size = pushSize;
            
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        }
            
        result = vkCreatePipelineLayout(
            device(), &pipelineLayoutInfo, nullptr, &m_pipeline_layout);
        VK_CHECK(result);
    }

    void NDIView::prepare_descriptor_pool()
    {
        if (m_desc_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device(), m_desc_pool, nullptr);
            m_desc_pool = VK_NULL_HANDLE;
        }
        
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount =
            static_cast<uint32_t>(m_textures.size()); // One per texture

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        VkResult result =
            vkCreateDescriptorPool(device(), &poolInfo, nullptr, &m_desc_pool);
        VK_CHECK(result);
    }

    void NDIView::prepare_descriptor_set()
    {
        
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_desc_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_desc_layout;

        VkResult result =
            vkAllocateDescriptorSets(device(), &allocInfo, &m_desc_set);
        VK_CHECK(result);

        std::vector<VkDescriptorImageInfo> imageInfos(m_textures.size());
        std::vector<VkWriteDescriptorSet> writes(m_textures.size());
        for (uint32_t i = 0; i < m_textures.size(); ++i)
        {
            imageInfos[i].sampler = m_textures[i]->getSampler();
            imageInfos[i].imageView = m_textures[i]->getImageView();
            imageInfos[i].imageLayout =
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = m_desc_set;
            writes[i].dstBinding = i; // 0 for main, 1+ for libplacebo
            writes[i].descriptorCount = 1;
            writes[i].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].pImageInfo = &imageInfos[i];
        }

        vkUpdateDescriptorSets(
            device(), static_cast<uint32_t>(writes.size()), writes.data(), 0,
            nullptr);
    }

    void NDIView::_exitThreads()
    {
        TLRENDER_P();

        p.videoThread.running = false;
        if (p.videoThread.thread.joinable())
            p.videoThread.thread.join();

        p.audioThread.running = false;
        if (p.audioThread.thread.joinable())
            p.audioThread.thread.join();
    }

    void NDIView::_init()
    {
        TLRENDER_P();

        // Some Vulkan settings
        m_clearColor = {2.F, 2.F, 2.F, 0.F};

        mode(FL_RGB | FL_DOUBLE | FL_ALPHA);

        if (!NDIlib_initialize())
            throw std::runtime_error("Could not initialize NDI library");

        p.placeboData.reset(new LibPlaceboData);

        p.NDISources = observer::List<std::string>::create();

        p.NDISourcesObserver = observer::ListObserver<std::string>::create(
            this->observeNDISources(),
            [this](const std::vector<std::string>& sources)
                {
                    TLRENDER_P();

                    fill_menu(HDRApp::ui->uiMenuBar);
                },
            observer::CallbackAction::Suppress);

        p.findThread.running = true;
        p.findThread.thread = std::thread(
            [this]
                {
                    try
                    {
                        _findThread();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                });
    }

    std::shared_ptr<observer::IList<std::string> >
    NDIView::observeNDISources() const
    {
        return _p->NDISources;
    }

    void NDIView::_startThreads()
    {
        TLRENDER_P();

        p.videoThread.running = true;
        p.videoThread.thread = std::thread([this] { _videoThread(); });
    }

    void NDIView::prepare()
    {
        TLRENDER_P();

        vkDeviceWaitIdle(device()); // waits for all queue on the device

        {
            std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
            m_textures.clear();
            prepare_main_texture();
            prepare_shader();
        }
        prepare_vertices();
        prepare_descriptor_layout();
        prepare_render_pass();
        prepare_pipeline();
        prepare_descriptor_pool();
        prepare_descriptor_set();

        m_swapchain_needs_recreation = false;
    }

    void NDIView::update_texture(VkCommandBuffer cmd)
    {
        TLRENDER_P();

        VkResult result;

        if (p.hdrMonitorFound && p.hasHDR && p.useHDRMetadata)
        {
            // This will make the FLTK swapchain call vk->SetHDRMetadataEXT();
            const image::HDRData& data = p.hdrData;
            auto m_previous_hdr_metadata = m_hdr_metadata;

            m_hdr_metadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
            m_hdr_metadata.displayPrimaryRed = {
                data.primaries[image::HDRPrimaries::Red][0],
                data.primaries[image::HDRPrimaries::Red][1],
            };
            m_hdr_metadata.displayPrimaryGreen = {
                data.primaries[image::HDRPrimaries::Green][0],
                data.primaries[image::HDRPrimaries::Green][1],
            };
            m_hdr_metadata.displayPrimaryBlue = {
                data.primaries[image::HDRPrimaries::Blue][0],
                data.primaries[image::HDRPrimaries::Blue][1],
            };
            m_hdr_metadata.whitePoint = {
                data.primaries[image::HDRPrimaries::White][0],
                data.primaries[image::HDRPrimaries::White][1],
            };
            // Max display capability
            m_hdr_metadata.maxLuminance =
                data.displayMasteringLuminance.getMax();
            m_hdr_metadata.minLuminance =
                data.displayMasteringLuminance.getMin();
            m_hdr_metadata.maxContentLightLevel = data.maxCLL;
            m_hdr_metadata.maxFrameAverageLightLevel = data.maxFALL;

            if (!is_equal_hdr_metadata(m_hdr_metadata, m_previous_hdr_metadata))
                m_hdr_metadata_changed = true; // Mark as changed
        }
        else
        {
            m_hdr_metadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;

            // Primaries
            m_hdr_metadata.displayPrimaryRed = { 0.640F, 0.330F };
            m_hdr_metadata.displayPrimaryGreen = { 0.300F, 0.600F };
            m_hdr_metadata.displayPrimaryBlue = { 0.15F, 0.060F };
            m_hdr_metadata.whitePoint = { 0.3127F, 0.3290F };
                
            // Max display capability
            m_hdr_metadata.maxLuminance = 100.F;
            m_hdr_metadata.minLuminance = 0.1F;
            m_hdr_metadata.maxContentLightLevel = 100.F;
            m_hdr_metadata.maxFrameAverageLightLevel = 100.F;
        }

        if (!p.image)
            return;

        // Compare the texture data size to the new image data size to see
        // if user changed streaming mid-way.
        const std::size_t dataSize = m_textures[0]->getWidth() *
                                     m_textures[0]->getHeight() *
                                     4 * sizeof(half);
        const std::size_t imageSize = p.image->getDataByteCount();
        if (dataSize != imageSize)
        {
            m_swapchain_needs_recreation = true;
        }
        else
        {
            m_textures[0]->copy(reinterpret_cast<const uint8_t*>(p.image->getData()), imageSize);
        }
    }
    
    bool NDIView::vk_draw_begin()
    {
        // Change background color here
        m_clearColor = {0.0, 0.0, 0.0, 0.0};
        return Fl_Vk_Window::vk_draw_begin();
    }

    void NDIView::draw()
    {
        TLRENDER_P();
        
        // Check if the window changed screen.
        bool changed_screen = false;
        if (p.screen_index != this->screen_num())
        {
            p.screen_index = this->screen_num();
            changed_screen = true;
        }

        if (changed_screen)
        {
            // If we changed screen from an HDR to an SDR one, or one with
            // different nits settings, recreate the Vulkan swapchain.
            auto hdr = getHDRCapabilities(this->screen_num());
            if (hdr.supported != p.hdrCapabilities.supported ||
                hdr.min_nits != p.hdrCapabilities.min_nits ||
                hdr.max_nits != p.hdrCapabilities.max_nits)
            {
                m_swapchain_needs_recreation = true;
                redraw();
                return;
            }
        }

        VkCommandBuffer cmd = getCurrentCommandBuffer();
        
        // Clear the frame
        begin_render_pass(cmd);
        end_render_pass(cmd);


        {
            std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
            update_texture(cmd);
            _fillVariables(cmd, p.placeboData->res);
        }
        
        begin_render_pass(cmd);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1,
            &m_desc_set, 0, nullptr);

        VkViewport viewport = {};
        viewport.width = static_cast<float>(w());
        viewport.height = static_cast<float>(h());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = w();
        scissor.extent.height = h();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // Draw the rectangle
        p.vao->draw(cmd, p.vbo);

        end_render_pass(cmd);
    }

    std::vector<const char*> NDIView::get_instance_extensions()
    {
        std::vector<const char*> out;
        out = Fl_Vk_Window::get_instance_extensions();
        return out;
    }

    std::vector<const char*> NDIView::get_optional_extensions()
    {
        std::vector<const char*> out;
        out = Fl_Vk_Window::get_optional_extensions();
        return out;
    }

    std::vector<const char*> NDIView::get_device_extensions()
    {
        std::vector<const char*> out;
        out = Fl_Vk_Window::get_device_extensions();
        out.push_back(VK_EXT_HDR_METADATA_EXTENSION_NAME);
        return out;
    }

    void NDIView::_getMonitorNits(bool quiet)
    {
        TLRENDER_P();
            
        if (!p.hdrCapabilities.supported)
        {
#ifdef __linux__
            if (!quiet)
            {
                std::cerr << _("Could not determine monitor's nits.")
                          << std::endl;
            }
            if (p.hdrMonitorFound)
            {
                p.hdrCapabilities.min_nits = 0.F;
                p.hdrCapabilities.max_nits = 1000.F;
            }
            else
            {
                p.hdrCapabilities.min_nits = 0.F;
                p.hdrCapabilities.max_nits = 100.F;
            }
#else
            p.hdrMonitorFound = false;
#endif
        }
            
        if (!p.hdrMonitorFound)
        {
            std::cout << _("HDR monitor not found or not configured.")
                      << std::endl;
            colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            format() = VK_FORMAT_B8G8R8A8_UNORM;
        }
        else
        {
            if (!quiet)
            {
                std::string msg =
                    string::Format(_("HDR monitor min. nits = {0}")).
                    arg(p.hdrCapabilities.min_nits);
                std::cout << msg << std::endl;
                
                msg = string::Format(_("HDR monitor max. nits = {0}")).
                      arg(p.hdrCapabilities.max_nits);
                std::cout << msg << std::endl;
            }
        }
    }


    void NDIView::_videoThread()
    {
        TLRENDER_P();
        while (p.videoThread.running)
        {
            std::unique_lock<std::mutex> lock(p.findMutex.mutex);
            if (p.currentNDISource.empty())
                continue;
            // We now have at least one source, so we create a receiver to
            // look at it.

            NDIlib_recv_create_t NDI_recv_create_desc;
            NDI_recv_create_desc.p_ndi_recv_name = "mrv2 HDR Receiver";
            NDI_recv_create_desc.source_to_connect_to =
                p.currentNDISource.c_str();
            NDI_recv_create_desc.color_format = NDIlib_recv_color_format_best;

            // Create the receiver
            p.NDI_recv = NDIlib_recv_create(&NDI_recv_create_desc);
            if (!p.NDI_recv)
            {
                continue;
            }

            lock.unlock(); // Explicitly unlock after creating receiver

            // The descriptors
            NDIlib_video_frame_t video_frame;
            NDIlib_audio_frame_t audio_frame; // not used yet

            while (p.videoThread.running)
            {

                switch (NDIlib_recv_capture(
                    p.NDI_recv, &video_frame, nullptr, nullptr, 1000))
                {
                    // Video data
                case NDIlib_frame_type_video:
                {
                    bool init = false;

                    float pixelAspectRatio = 1.F;
                    if (video_frame.picture_aspect_ratio == 0.F)
                        pixelAspectRatio =
                            1.F / (video_frame.xres * video_frame.yres);

                    if (p.info.size.w != video_frame.xres ||
                        p.info.size.h != video_frame.yres ||
                        p.info.size.pixelAspectRatio != pixelAspectRatio)
                    {
                        init = true;
                    }

                    if (video_frame.p_metadata)
                    {
                        // Parsing XML metadata
                        try
                        {
                            rapidxml::xml_document<> doc;
                            doc.parse<0>((char*)video_frame.p_metadata);

                            // Get root node
                            rapidxml::xml_node<>* root =
                                doc.first_node("ndi_color_info");

                            // Get attributes
                            if (root)
                            {
                                rapidxml::xml_attribute<>* attr_mrv2 =
                                    root->first_attribute("mrv2");

                                if (!p.hasHDR)
                                    init = true;
                                p.hasHDR = true;

                                rapidxml::xml_attribute<>* attr_transfer =
                                    root->first_attribute("transfer");
                                if (attr_transfer)
                                {
                                    p.transferName = attr_transfer->value();

                                    if (p.hdrMonitorFound &&
                                        colorSpace() !=
                                            VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
                                    {
                                        if (p.transferName == "bt_2100_hlg")
                                        {
                                            colorSpace() =
                                                VK_COLOR_SPACE_HDR10_HLG_EXT;
                                        }
                                        else
                                        {
                                            colorSpace() =
                                                VK_COLOR_SPACE_HDR10_ST2084_EXT;
                                        }

                                        if (p.lastColorSpace != colorSpace())
                                        {
                                            p.lastColorSpace = colorSpace();
                                            m_swapchain_needs_recreation = true;
                                        }
                                    }
                                }

                                image::HDRData hdrData;
                                if (attr_mrv2)
                                {
                                    const std::string& jsonString =
                                        unescape_quotes_from_xml(
                                            attr_mrv2->value());

                                    const nlohmann::json& j =
                                        nlohmann::json::parse(jsonString);

                                    hdrData = j.get<image::HDRData>();
                                    if (!image::isHDR(hdrData))
                                    {
                                        p.hasHDR = false;
                                        init = true;
                                    }
                                }
                                else
                                {
                                    rapidxml::xml_attribute<>* attr_matrix =
                                        root->first_attribute("matrix");
                                    rapidxml::xml_attribute<>* attr_primaries =
                                        root->first_attribute("primaries");

                                    if (attr_matrix)
                                        p.matrixName = attr_matrix->value();
                                    if (attr_primaries)
                                        p.primariesName =
                                            attr_primaries->value();
                                }

                                if (p.hdrData != hdrData)
                                {
                                    p.hdrData = hdrData;
                                }
                            }
                            else
                            {
                                if (p.hasHDR)
                                    init = true;
                                p.hasHDR = false;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "hdr error: " << e.what()
                                      << std::endl;
                        }
                    }
                    else
                    {
                        if (p.hasHDR)
                            init = true;
                        p.hasHDR = false;
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        if (init)
                        {
                            p.info.size.w = video_frame.xres;
                            p.info.size.h = video_frame.yres;
                            p.info.size.pixelAspectRatio = pixelAspectRatio;
                            p.info.layout.mirror.y = true;
                            p.info.pixelType = image::PixelType::RGBA_F16;
                            p.info.videoLevels = image::VideoLevels::FullRange;
                            p.fourCC = video_frame.FourCC;
                            p.image = image::Image::create(p.info);
                            m_swapchain_needs_recreation = true;
                        }
                        else if (video_frame.p_data)
                        {
                            _copy(video_frame.p_data,
                                  video_frame.line_stride_in_bytes);
                            redraw();
                        }
                    }

                    NDIlib_recv_free_video(p.NDI_recv, &video_frame);
                    break;

                    //     // Audio data
                    // case NDIlib_frame_type_audio:
                    //     printf("Audio data received (%d samples).\n",
                    //     audio_frame.no_samples);
                    //     NDIlib_recv_free_audio(p.NDI_recv, &audio_frame);
                    //     break;

                    // Everything else
                }
                default:
                    break;
                }
            }
        }

        // Destroy the receiver
        NDIlib_recv_destroy(p.NDI_recv);
    }

    void NDIView::_findThread()
    {
        TLRENDER_P();

        const NDIlib_source_t* sources = nullptr;

        p.NDI_find = NDIlib_find_create_v2();
        if (!p.NDI_find)
            throw std::runtime_error("could not create ndi find");

        while (p.findThread.running)
        {
            using namespace std::chrono;
            for (const auto startTime = high_resolution_clock::now();
                 high_resolution_clock::now() - startTime < seconds(3);)
            {
                // Wait up till 1 second to check for new sources to be added or
                // removed
                if (!NDIlib_find_wait_for_sources(
                        p.NDI_find, 1000 /* milliseconds */))
                {
                    break;
                }
            }

            uint32_t no_sources = 0;

            std::unique_lock lock(p.findMutex.mutex);

            // Get the updated list of sources
            while (!no_sources)
            {
                sources =
                    NDIlib_find_get_current_sources(p.NDI_find, &no_sources);
            }

            static std::regex kRemoteRegex(
                ".*REMOTE.*", std::regex_constants::icase);

            std::vector<std::string> NDIsources;
            for (int i = 0; i < no_sources; ++i)
            {
                if (std::regex_match(sources[i].p_ndi_name, kRemoteRegex))
                    continue;
                NDIsources.push_back(sources[i].p_ndi_name);
            }

            p.NDISources->setIfChanged(NDIsources);

            if (NDIsources.size() == 1)
            {
                setNDISource(NDIsources[0]);
            }
        }
    }

    void NDIView::destroy_textures()
    {
        m_textures.clear();
    }

    void NDIView::destroy()
    {
        TLRENDER_P();


        p.shader.reset();
        
        p.vao.reset();
        p.vbo.reset();

        destroy_textures();

        if (m_pipeline_layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device(), m_pipeline_layout, nullptr);
            m_pipeline_layout = VK_NULL_HANDLE;
        }

        if (m_desc_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device(), m_desc_layout, nullptr);
            m_desc_layout = VK_NULL_HANDLE;
        }

        if (m_desc_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device(), m_desc_pool, nullptr);
            m_desc_pool = VK_NULL_HANDLE;
        }
    }

    void NDIView::_parseVariables(std::stringstream& s,
                                  std::size_t& currentOffset,
                                  const struct pl_shader_res* res,
                                  const std::size_t pushConstantsMaxSize)
    {
        TLRENDER_P();
            
        // Collect non-floats and floats separately to optimize push constants
        std::vector<struct pl_shader_var> non_floats;
        std::vector<struct pl_shader_var> floats;
    
        for (int i = 0; i < res->num_variables; ++i) {
            const struct pl_shader_var shader_var = res->variables[i];
            const struct pl_var var = shader_var.var;
            const std::string glsl_type = pl_var_glsl_type_name(var);
            const bool is_float = (glsl_type == "float");
        
            if (is_float) {
                floats.push_back(shader_var);
            } else {
                non_floats.push_back(shader_var);
            }
        }
    
        // Combine into a grouped list: non-floats first, then floats
        std::vector<struct pl_shader_var> all_grouped;
        all_grouped.reserve(non_floats.size() + floats.size());
        all_grouped.insert(all_grouped.end(), non_floats.begin(), non_floats.end());
        all_grouped.insert(all_grouped.end(), floats.begin(), floats.end());
    
        // Now pack into push constants until we can't fit more
        std::vector<struct pl_shader_var> push_vars;
        std::vector<struct pl_shader_var> ubo_vars;
    
        for (const auto &shader_var : all_grouped)
        {
            const struct pl_var var = shader_var.var;
            const struct pl_var_layout layout = pl_std430_layout(currentOffset,
                                                                 &var);
        
            if (layout.offset + layout.size > pushConstantsMaxSize) {
                ubo_vars.push_back(shader_var);
            } else {
                push_vars.push_back(shader_var);
                currentOffset = layout.offset + layout.size;
            }
        }
    
        // Generate push constant block if there are variables for it
        if (!push_vars.empty())
        {
            s << "layout(std430, push_constant) uniform PushC {\n";
            size_t offset = 0;
            for (const auto &shader_var : push_vars)
            {
                const struct pl_var var = shader_var.var;
                const std::string glsl_type = pl_var_glsl_type_name(var);
                const struct pl_var_layout layout = pl_std430_layout(offset, &var);
                s << "\tlayout(offset=" << layout.offset << ") " << glsl_type << " " << var.name << ";\n";
                offset = layout.offset + layout.size;
            }
            s << "};\n";
        }
    
        // Generate UBO block if there are remaining variables
        if (!ubo_vars.empty())
        {
            s << "layout(std140, binding=" << p.bindingIndex++ << ") uniform pcUBO {\n";
            size_t offset = 0;
            for (const auto &shader_var : ubo_vars) {
                const struct pl_var var = shader_var.var;
                const std::string glsl_type = pl_var_glsl_type_name(var);
                const struct pl_var_layout layout = pl_std140_layout(offset, &var);
                s << "\tlayout(offset=" << layout.offset << ") " << glsl_type << " "
                  << var.name << ";\n";
                offset = layout.offset + layout.size;
            }
            s << "};\n";

            // Create a unifor pcUBO parameter
            p.placeboData->pcUBOData = malloc(offset);
            p.placeboData->pcUBOSize = offset;
            memset(p.placeboData->pcUBOData, 0, offset);
        }
    }
            
    void NDIView::prepare_shader()
    {
        TLRENDER_P();

        if (!p.placeboData)
            p.placeboData.reset(new LibPlaceboData);

        // if (p.placeboData->shader)
        //     pl_shader_free(&p.placeboData->shader);
        
        pl_shader_params shader_params;
        memset(&shader_params, 0, sizeof(pl_shader_params));
                
        shader_params.id = 1;
        shader_params.gpu = p.placeboData->gpu;
        shader_params.dynamic_constants = false;
            
        pl_shader_reset(p.placeboData->shader, &shader_params);        

        pl_color_map_params cmap = pl_color_map_high_quality_params;

        cmap.tone_mapping_function = nullptr;

        const image::HDRData& data = p.hdrData;

        pl_color_space src_colorspace;
        memset(&src_colorspace, 0, sizeof(pl_color_space));

        if (p.hasHDR)
        {
            src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
            src_colorspace.transfer = PL_COLOR_TRC_PQ;
            if (p.transferName == "bt_2100_hlg")
            {
                src_colorspace.transfer = PL_COLOR_TRC_HLG;
            }

            cmap.metadata = PL_HDR_METADATA_ANY;
            pl_hdr_metadata& hdr = src_colorspace.hdr;
            hdr.min_luma = data.displayMasteringLuminance.getMin();
            hdr.max_luma = data.displayMasteringLuminance.getMax();
            hdr.prim.red.x = data.primaries[image::HDRPrimaries::Red][0];
            hdr.prim.red.y = data.primaries[image::HDRPrimaries::Red][1];
            hdr.prim.green.x = data.primaries[image::HDRPrimaries::Green][0];
            hdr.prim.green.y = data.primaries[image::HDRPrimaries::Green][1];
            hdr.prim.blue.x = data.primaries[image::HDRPrimaries::Blue][0];
            hdr.prim.blue.y = data.primaries[image::HDRPrimaries::Blue][1];
            hdr.prim.white.x = data.primaries[image::HDRPrimaries::White][0];
            hdr.prim.white.y = data.primaries[image::HDRPrimaries::White][1];
            hdr.max_cll = data.maxCLL;
            hdr.max_fall = data.maxFALL;
            hdr.scene_max[0] = data.sceneMax[0];
            hdr.scene_max[1] = data.sceneMax[1];
            hdr.scene_max[2] = data.sceneMax[2];
            hdr.scene_avg = data.sceneAvg;
            hdr.ootf.target_luma = data.ootf.targetLuma;
            hdr.ootf.knee_x = data.ootf.kneeX;
            hdr.ootf.knee_y = data.ootf.kneeY;
            hdr.ootf.num_anchors = data.ootf.numAnchors;
            for (int i = 0; i < hdr.ootf.num_anchors; i++)
                hdr.ootf.anchors[i] = data.ootf.anchors[i];
            hdr.max_pq_y = data.maxPQY;
            hdr.avg_pq_y = data.avgPQY;
        }
        else
        {
            src_colorspace.primaries = PL_COLOR_PRIM_BT_709;
            src_colorspace.transfer = PL_COLOR_TRC_BT_1886;
        }

        pl_color_space_infer(&src_colorspace);

        pl_color_space dst_colorspace;
        memset(&dst_colorspace, 0, sizeof(pl_color_space));

        if (p.hdrMonitorFound)
        {
            dst_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
            dst_colorspace.transfer = PL_COLOR_TRC_PQ;
            dst_colorspace.hdr.min_luma = p.hdrCapabilities.min_nits;
            dst_colorspace.hdr.max_luma = p.hdrCapabilities.max_nits;
            
            if (colorSpace() == VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
            {
                dst_colorspace.primaries = PL_COLOR_PRIM_DISPLAY_P3;
                dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;
            }
            else if (colorSpace() == VK_COLOR_SPACE_HDR10_HLG_EXT)
            {
                dst_colorspace.transfer = PL_COLOR_TRC_HLG;
            }
            else if (colorSpace() == VK_COLOR_SPACE_DOLBYVISION_EXT)
            {
                // \@todo:  How to handle this? PL_COLOR_TRC_DOLBYVISION does
                //          not exist.
                // dst_colorspace.transfer = ???
                dst_colorspace.transfer = PL_COLOR_TRC_PQ;
            }

            // For SDR content on HDR monitor, enable tone mapping to fit SDR
            // into HDR
            if (!p.hasHDR)
            {
                cmap.tone_mapping_function = &pl_tone_map_spline;
                cmap.metadata = PL_HDR_METADATA_NONE; // Simplify
            }
            else
            {
                cmap.tone_mapping_function = &pl_tone_map_spline;
            }
        }
        else
        {

            dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
            dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;

            dst_colorspace.hdr.min_luma = 0.F;
            dst_colorspace.hdr.max_luma = 203.0F; // SDR peak
                            
            if (p.hasHDR)
                cmap.tone_mapping_function = &pl_tone_map_spline;
            else
                cmap.tone_mapping_function = nullptr;
        }

        pl_color_space_infer(&dst_colorspace);

        pl_color_map_args color_map_args;
        memset(&color_map_args, 0, sizeof(pl_color_map_args));

        color_map_args.src = src_colorspace;
        color_map_args.dst = dst_colorspace;
        color_map_args.prelinearized = false;
        
        // Reuse results if possible
        color_map_args.state = &p.placeboData->state;

        pl_shader_color_map_ex(p.placeboData->shader, &cmap, &color_map_args);

        p.placeboData->res = pl_shader_finalize(p.placeboData->shader);
        const pl_shader_res* res = p.placeboData->res;
        if (!p.placeboData->res)
        {
            p.placeboData.reset();
            throw std::runtime_error("pl_shader_finalize failed!");
        }

        std::stringstream s;

        // std::cerr << "num_vertex_attribs=" << res->num_vertex_attribs
        //           << std::endl
        //           << "num_descriptors=" << res->num_descriptors << std::endl
        //           << "num_variables=" << res->num_variables << std::endl
        //           << "num_constants=" << res->num_constants << std::endl;

        for (int i = 0; i < res->num_descriptors; i++)
        {
            const pl_shader_desc* sd = &res->descriptors[i];
            const pl_desc* desc = &sd->desc;
            switch (desc->type)
            {
            case PL_DESC_SAMPLED_TEX:
            case PL_DESC_STORAGE_IMG:
            {
                static const char* types[] = {
                    "sampler1D",
                    "sampler2D",
                    "sampler3D",
                };

                pl_desc_binding binding = sd->binding;
                pl_tex tex = (pl_tex)binding.object;
                int dims = pl_tex_params_dimension(tex->params);
                const char* type = types[dims - 1];

                char prefix = ' ';
                switch (tex->params.format->type)
                {
                case PL_FMT_UINT:
                    prefix = 'u';
                    break;
                case PL_FMT_SINT:
                    prefix = 'i';
                    break;
                case PL_FMT_FLOAT:
                case PL_FMT_UNORM:
                case PL_FMT_SNORM:
                default:
                    break;
                }

                s << "layout(binding = " << (i + 1) << ") uniform " << prefix
                  << type << " " << desc->name << ";" << std::endl;
                break;
            }
            case PL_DESC_BUF_UNIFORM:
                throw "buf uniform";
                break;
            case PL_DESC_BUF_STORAGE:
                throw "buf storage";
            case PL_DESC_BUF_TEXEL_UNIFORM:
                throw "buf texel uniform";
            case PL_DESC_BUF_TEXEL_STORAGE:
                throw "buf texel storage";
            case PL_DESC_INVALID:
            case PL_DESC_TYPE_COUNT:
                throw "invalid or count";
                break;
            }
        }

        s << "//" << std::endl
          << "// Variables" << std::endl
          << "//" << std::endl
          << std::endl;

        size_t pushSize = 0;
        _parseVariables(s, pushSize, res,
                         ctx.gpu_props.limits.maxPushConstantsSize);

        s << std::endl
          << "//" << std::endl
          << "// Constants" << std::endl
          << "//" << std::endl
          << std::endl;
        for (int i = 0; i < res->num_constants; ++i)
        {
            // s << "layout(constant_id=" << i << ") ";
            const struct pl_shader_const constant = res->constants[i];
            switch (constant.type)
            {
            case PL_VAR_SINT:
                s << "const int " << constant.name << " = "
                  << *(reinterpret_cast<const int*>(constant.data));
                break;
            case PL_VAR_UINT:
                s << "const uint " << constant.name << " = "
                  << *(reinterpret_cast<const unsigned*>(constant.data));
                break;
            case PL_VAR_FLOAT:
                s << "const float " << constant.name << " = "
                  << *(reinterpret_cast<const float*>(constant.data));
                break;
            default:
                break;
            }
            s << ";" << std::endl;
        }

        s << res->glsl << std::endl;
        p.hdrColorsDef = s.str();

        try
        {
            if (m_textures.size() > 1)
            {
                // Remove all ements but the first, main image
                m_textures.resize(1);
            }
            addGPUTextures(res);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            p.placeboData.reset();
            return;
        }

        {
            p.hdrColors = "outColor = ";
            p.hdrColors += res->name;
            p.hdrColors += "(tmp);\n";
        }

        const std::string source = _fragmentSource();

        bool recreateShader = false;
        if (!p.shader || p.oldShaderSource != source)
        {
            recreateShader = true;
        }
        p.oldShaderSource = source;

        if (recreateShader)
        {
            p.shader = vlk::Shader::create(ctx, vertexSource(), source, "hdr");
            for (const auto& texture : m_textures)
            {
                p.shader->addTexture(texture->getName());
            }

            if (p.shader && p.placeboData->pcUBOSize > 0)
                 p.shader->createUniformData("pcUBO", p.placeboData->pcUBOSize);

            
            auto bindingSet = p.shader->createBindingSet();

            for (const auto& texture : m_textures)
            {
                p.shader->setTexture(texture->getName(), texture);
            }

            p.shader->createPush("placeboPC", pushSize, vlk::kShaderFragment);
        }
    }
    
    void NDIView::_fillVariables(VkCommandBuffer cmd, const struct pl_shader_res* res)
    {
        TLRENDER_P();
        
        if (!res)
            return;
        
        size_t pushSize = p.shader->getPushSize();
        if (pushSize > 0)
        {
            std::vector<uint8_t> pushData(pushSize, 0);

            VkPipelineLayout pipelineLayout = m_pipeline_layout;
            std::size_t currentOffset = 0;
            const pl_shader_res* res = p.placeboData->res;
            for (int j = 0; j < 2; ++j)
            {
                for (int i = 0; i < res->num_variables; ++i)
                {
                    const struct pl_shader_var& shader_var = res->variables[i];
                    const struct pl_var& var = shader_var.var;
                    const std::string glsl_type = pl_var_glsl_type_name(var);
                    const bool is_float = (glsl_type == "float");
                    if (j == 0 && is_float)
                        continue;
                    if (j == 1 && !is_float)
                        continue;
                            
                    // Ensure the variable type is float-based
                    if (var.type != PL_VAR_FLOAT)
                    {
                        throw std::runtime_error("libplacebo created a variable that is not float");
                    }
                            
                    const struct pl_var_layout& dst_layout = pl_std430_layout(currentOffset, &var);
                    const struct pl_var_layout& src_layout = pl_var_host_layout(0, &var);
                            
                    memcpy_layout(pushData.data(), dst_layout,
                                  shader_var.data, src_layout);
                    currentOffset = dst_layout.offset + dst_layout.size;
                }
            }
                    
            vkCmdPushConstants(cmd, pipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               pushData.size(), pushData.data());
        }

        if (p.placeboData && p.placeboData->pcUBOSize > 0)
        {
            std::size_t currentOffset = 0;
            for (const auto &shader_var : p.placeboData->pcUBOvars)
            {
                const struct pl_var var = shader_var.var;
                const struct pl_var_layout dst_layout = pl_std140_layout(currentOffset, &var);
                const struct pl_var_layout& src_layout = pl_var_host_layout(0, &var);

                memcpy_layout(p.placeboData->pcUBOData, dst_layout,
                              shader_var.data, src_layout);

                currentOffset = dst_layout.offset + dst_layout.size;
            }
                
            p.shader->setUniformData("pcUBO", p.placeboData->pcUBOData,
                                     p.placeboData->pcUBOSize);
        }
    }

    void NDIView::setNDISource(const std::string& source)
    {
        TLRENDER_P();

        if (p.currentNDISource != source)
        {
            std::cerr << "Changing source to " << source << std::endl;
            p.currentNDISource = source;

            HDRUI* ui = HDRApp::ui;
            fill_menu(ui->uiMenuBar);

            _exitThreads();
            _startThreads();
        }
    }

    int NDIView::handle(int event)
    {
        switch (event)
        {
        case FL_PUSH:
            if (Fl::event_button3())
            {
                Fl_Group::current(0);

                Fl_Menu_Button* popupMenu = new Fl_Menu_Button(0, 0, 0, 0);

                popupMenu->textsize(12);
                popupMenu->type(Fl_Menu_Button::POPUP3);

                fill_menu(popupMenu);
                popupMenu->popup();

                delete popupMenu;
                popupMenu = nullptr;
                return 1;
            }
        default:
            return Fl_Vk_Window::handle(event);
            return 0;
        }
    }

    void NDIView::fill_menu(Fl_Menu_* menu)
    {
        TLRENDER_P();

        int idx;
        int mode = 0;
        HDRUI* ui = HDRApp::ui;
        Fl_Menu_Item* item = nullptr;

        menu->clear();
        const auto& sources = p.NDISources->get();

        mode = FL_MENU_RADIO;

        for (auto& source : sources)
        {
            std::string entry = _("Connection/");
            entry += source;
            idx = menu->add(
                entry.c_str(), 0, (Fl_Callback*)select_ndi_source_cb, ui, mode);
            if (source == p.currentNDISource)
            {
                item = (Fl_Menu_Item*)&menu->menu()[idx];
                item->set();
            }
        }

        mode = FL_MENU_TOGGLE;
        idx = menu->add(
            _("Window/Fullscreen"), 0, (Fl_Callback*)toggle_fullscreen_cb, ui,
            mode);
        if (ui->uiMain->fullscreen_active())
        {
            item = (Fl_Menu_Item*)&menu->menu()[idx];
            item->set();
        }

        mode = FL_MENU_TOGGLE;
        idx = menu->add(
            _("HDR/Pass through Metadata to Display"), 0,
            (Fl_Callback*)apply_metadata_cb, ui, mode);
        if (p.useHDRMetadata)
        {
            item = (Fl_Menu_Item*)&menu->menu()[idx];
            item->set();
        }

        menu->menu_end();
        menu->redraw();
    }

    void NDIView::toggle_hdr_metadata()
    {
        TLRENDER_P();
        p.useHDRMetadata = !p.useHDRMetadata;
        m_swapchain_needs_recreation = true;

        if (p.useHDRMetadata && !p.isP3Display)
        {
            if (p.transferName == "bt_2100_hlg")
            {
                colorSpace() = VK_COLOR_SPACE_HDR10_HLG_EXT;
            }
            else
            {
                colorSpace() = VK_COLOR_SPACE_HDR10_ST2084_EXT;
            }
        }

        if (p.lastColorSpace != colorSpace())
        {
            p.lastColorSpace = colorSpace();
        }

        redraw();
    }

    void NDIView::toggle_fullscreen()
    {
        TLRENDER_P();

        HDRUI* ui = HDRApp::ui;

        if (ui->uiMain->fullscreen_active())
        {
            ui->uiMain->fullscreen_off();
        }
        else
        {
            ui->uiMain->fullscreen();
        }
    }

    std::string NDIView::_fragmentSource()
    {
        TLRENDER_P();
        
        std::string fragShader = tl::string::Format(R"(
#version 450

// Input from vertex shader
layout(location = 0) in vec2 inTexCoord;

// Output color
layout(location = 0) out vec4 outColor;

// Main image to display
layout(binding = 0) uniform sampler2D inTexture;
{0}

void main() {
     vec4 tmp = texture(inTexture, inTexCoord, 0.0);
     {1}
}
)").arg(p.hdrColorsDef).arg(p.hdrColors);
        return fragShader;
    }

} // namespace mrv
