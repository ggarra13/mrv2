// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Util.h>

#include <tlGL/GL.h>
#include <tlGL/Init.h>

#include <tlCore/Assert.h>

#include <array>
#include <cctype>
#include <map>
#include <mutex>
#include <sstream>
#include <tuple>

namespace tl
{
    namespace gl
    {
        bool hasExtension(const std::string& value)
        {
            static std::mutex mutex;
            static std::map<std::string, bool> cache;
            std::lock_guard<std::mutex> lock(mutex);
            const auto i = cache.find(value);
            if (i != cache.end())
            {
                return i->second;
            }
            bool out = false;
            GLint count = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &count);
            for (GLint j = 0; j < count && !out; ++j)
            {
                const GLubyte* extension = glGetStringi(GL_EXTENSIONS, j);
                out = extension && value == reinterpret_cast<const char*>(extension);
            }
            cache[value] = out;
            return out;
        }


        namespace
        {
            constexpr std::size_t pixelTypeCount =
                static_cast<std::size_t>(image::PixelType::Count);

            //! Build a lookup table indexed by image::PixelType. The size is
            //! checked against image::PixelType::Count at compile time so a
            //! table can never silently fall out of sync with the enum.
            template <typename... Args>
            constexpr std::array<GLenum, sizeof...(Args)> makeTable(Args... args)
            {
                return {{static_cast<GLenum>(args)...}};
            }
        } // namespace

        unsigned int getReadPixelsFormat(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_RED, GL_RED, GL_RED, GL_RED, GL_RED,
                // LA_*
                GL_RG, GL_RG, GL_RG, GL_RG, GL_RG,
                // RGB_U8, RGB_U10 (packed, needs RGBA), RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_RGB, GL_RGB, GL_RGB, GL_RGB,
                // RGBA_*
                GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA,
                // YUV planar (read back per plane as L_*)
                GL_NONE, GL_NONE, GL_NONE,  // U8

                GL_NONE, GL_NONE, GL_NONE,  // U10

                GL_NONE, GL_NONE, GL_NONE,  // U12

                GL_NONE, GL_NONE, GL_NONE,  // U16

                // YUV semi-planar
                GL_NONE, GL_NONE,

                GL_NONE, GL_NONE,

                GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_BGRA,
                // RGB_F16_P (planar, cannot be read in one call)
                GL_NONE);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_U8 (implementation defined), L_U16, L_U32, L_F16, L_F32
                GL_RED, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // LA_*
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // RGB_U8 (implementation defined), RGB_U10, RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // RGBA_U8 (always supported), RGBA_U16, RGBA_U32, RGBA_F16, RGBA_F32 (float color buffers)
                GL_RGBA, GL_NONE, GL_NONE, GL_RGBA, GL_RGBA,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,  // U8
                GL_NONE, GL_NONE, GL_NONE,  // U10
                GL_NONE, GL_NONE, GL_NONE,  // U12
                GL_NONE, GL_NONE, GL_NONE,  // U16
                // YUV semi-planar
                GL_NONE, GL_NONE,

                GL_NONE, GL_NONE,

                GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_NONE);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        unsigned int getReadPixelsType(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // LA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_UNSIGNED_SHORT_4_4_4_4_REV,
                // RGB_F16_P
                GL_NONE);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // LA_*
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_2_10_10_10_REV, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_NONE);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        struct SetAndRestore::Private
        {
            unsigned int id = 0;
            GLboolean previous = GL_FALSE;
        };

        SetAndRestore::SetAndRestore(unsigned int id, bool value) :
            _p(new Private)
        {
            _p->id = id;

            glGetBooleanv(id, &_p->previous);

            if (value)
            {
                glEnable(id);
            }
            else
            {
                glDisable(id);
            }
        }

        SetAndRestore::~SetAndRestore()
        {
            if (_p->previous)
            {
                glEnable(_p->id);
            }
            else
            {
                glDisable(_p->id);
            }
        }

        std::string getErrorLabel(unsigned int value)
        {
            std::string out;
            switch (value)
            {
            case GL_NO_ERROR:
                out = "GL_NO_ERROR";
                break;
            case GL_INVALID_ENUM:
                out = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                out = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                out = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                out = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                out = "GL_OUT_OF_MEMORY";
                break;
            default:
                break;
            }
            return out;
        }
    } // namespace gl
} // namespace tl
