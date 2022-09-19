// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>


#include <tlGlad/gl.h>

// mrViewer includes
#include <mrvCore/mrvSequence.h>
#include <mrvFl/mrvIO.h>
#include <mrvFl/mrvTimelinePlayer.h>
#include <mrViewer.h>

#include <mrvGL/mrvTimelineViewport.h>
#include <mrvGL/mrvTimelineViewportPrivate.h>
#include <mrvGL/mrvGLViewport.h>

#include <glm/gtc/matrix_transform.hpp>

// For main fltk event loop
#include <FL/Fl.H>

//! Define a variable, "p", that references the private implementation.
#define TLRENDER_GL()                           \
    auto& gl = *_gl

namespace {
    const char* kModule = "glview";
}

namespace mrv
{
    using namespace tl;

    struct GLViewport::GLPrivate
    {
        std::weak_ptr<system::Context> context;

        // GL variables
        //! OpenGL Offscreen buffer
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer = nullptr;
        std::shared_ptr<tl::gl::Render> render = nullptr;
        std::shared_ptr<tl::gl::Shader> shader    = nullptr;
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;
    };


    GLViewport::GLViewport( int X, int Y, int W, int H, const char* L ) :
        TimelineViewport( X, Y, W, H, L ),
        _gl( new GLPrivate )
    {
        mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | FL_OPENGL3 );
    }


    GLViewport::~GLViewport()
    {
    }

    void GLViewport::setContext(
        const std::weak_ptr<system::Context>& context )
    {
        _gl->context = context;
    }

    void GLViewport::initializeGL()
    {
        TLRENDER_P();
        TLRENDER_GL();
        try
        {
            gladLoaderLoadGL();
            if ( !gl.render )
            {
                if (auto context = gl.context.lock())
                {
                    gl.render = gl::Render::create(context);
                    p.fontSystem = imaging::FontSystem::create(context);
                }
            }

            if ( !gl.shader )
            {

            const std::string vertexSource =
                "#version 410\n"
                "\n"
                "in vec3 vPos;\n"
                "in vec2 vTexture;\n"
                "out vec2 fTexture;\n"
                "\n"
                "uniform struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "} transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fTexture = vTexture;\n"
                "}\n";
            const std::string fragmentSource =
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    fColor = texture(textureSampler, fTexture);\n"
                "}\n";
            gl.shader = gl::Shader::create(vertexSource, fragmentSource);
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "mrv::GLViewport",
                    e.what(),
                    log::Type::Error);
            }
        }
    }



    void GLViewport::draw()
    {
        TLRENDER_P();
        TLRENDER_GL();

        if ( !valid() )
        {
            initializeGL();
            valid(1);
        }


        const auto renderSize = _getRenderSize();
        try
        {
            if (renderSize.isValid())
            {
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorMinifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.minify);
                    offscreenBufferOptions.colorMagnifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.magnify);
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(gl.buffer, renderSize, offscreenBufferOptions))
                {
                    gl.buffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                }
            }
            else
            {
                gl.buffer.reset();
            }

            if (gl.buffer)
            {
                gl::OffscreenBufferBinding binding(gl.buffer);
                gl.render->setColorConfig(p.colorConfigOptions);
                gl.render->begin(renderSize);
                gl.render->drawVideo(
                    p.videoData,
                    timeline::tiles(p.compareOptions.mode,
                                    _getTimelineSizes()),
                    p.imageOptions,
                    p.displayOptions,
                    p.compareOptions);
                gl.render->end();
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "mrv::GLViewport",
                    e.what(),
                    log::Type::Error);
            }
        }

        const auto viewportSize = _getViewportSize();
        glViewport(
            0,
            0,
            GLsizei(viewportSize.w),
            GLsizei(viewportSize.h));
        glClearColor(0.0F, 0.F, 0.F, 1.F);
        glClear(GL_COLOR_BUFFER_BIT);

        if (gl.buffer)
        {
            gl.shader->bind();
            glm::mat4x4 vm(1.F);
            vm = glm::translate(vm, glm::vec3(p.viewPos.x, p.viewPos.y, 0.F));
            vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));
            const glm::mat4x4 pm = glm::ortho(
                0.F,
                static_cast<float>(viewportSize.w),
                0.F,
                static_cast<float>(viewportSize.h),
                -1.F,
                1.F);
            glm::mat4x4 vpm = pm * vm;
            auto mvp = math::Matrix4x4f(
                vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3] );

            gl.shader->setUniform("transform.mvp", mvp);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gl.buffer->getColorID());

            geom::TriangleMesh3 mesh;
            mesh.v.push_back(math::Vector3f(0.F, 0.F, 0.F));
            mesh.t.push_back(math::Vector2f(0.F, 0.F));
            mesh.v.push_back(math::Vector3f(renderSize.w, 0.F, 0.F));
            mesh.t.push_back(math::Vector2f(1.F, 0.F));
            mesh.v.push_back(math::Vector3f(renderSize.w, renderSize.h, 0.F));
            mesh.t.push_back(math::Vector2f(1.F, 1.F));
            mesh.v.push_back(math::Vector3f(0.F, renderSize.h, 0.F));
            mesh.t.push_back(math::Vector2f(0.F, 1.F));
            mesh.triangles.push_back(geom::Triangle3({
                        geom::Vertex3({ 1, 1, 0 }),
                        geom::Vertex3({ 2, 2, 0 }),
                        geom::Vertex3({ 3, 3, 0 })
                    }));
            mesh.triangles.push_back(geom::Triangle3({
                        geom::Vertex3({ 3, 3, 0 }),
                        geom::Vertex3({ 4, 4, 0 }),
                        geom::Vertex3({ 1, 1, 0 })
                    }));

            auto vboData = convert(
                mesh,
                gl::VBOType::Pos3_F32_UV_U16,
                math::SizeTRange(0, mesh.triangles.size() - 1));
            if (!gl.vbo)
            {
                gl.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
            }
            if (gl.vbo)
            {
                gl.vbo->copy(vboData);
            }

            if (!gl.vao && gl.vbo)
            {
                gl.vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, gl.vbo->getID());
            }
            if (gl.vao && gl.vbo)
            {
                gl.vao->bind();
                gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
                _updatePixelBar();
            }
            if ( p.hud != HudDisplay::kNone ) _drawHUD();
        }


    }


    inline
    void GLViewport::_drawText( const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
                                math::Vector2i& pos,
                                const int16_t lineHeight,
                                const imaging::Color4f& labelColor)
    {
        TLRENDER_GL();
        const imaging::Color4f shadowColor(0.F, 0.F, 0.F, 0.7F);
        math::Vector2i shadowPos{ pos.x + 2, pos.y + 2 };
        gl.render->drawText( glyphs, shadowPos,shadowColor );
        gl.render->drawText( glyphs, pos, labelColor );
        pos.y += lineHeight;
    }

    void GLViewport::_readPixel( imaging::Color4f& rgba ) const noexcept
    {
        if ( !valid() ) return;

        TLRENDER_P();

        timeline::Playback playback = p.timelinePlayers[0]->playback();

        // When playback is stopped we read the pixel from the front
        // buffer.  When it is olaying, we read it from the back buffer.
        if ( playback == timeline::Playback::Stop )
            glReadBuffer( GL_FRONT );
        else
            glReadBuffer( GL_BACK );

        glPixelStorei(GL_PACK_ALIGNMENT, 1);


        const GLenum format = GL_RGBA;
        const GLenum type = GL_FLOAT;

        glReadPixels( p.mousePos.x, p.mousePos.y, 1, 1,
                      format, type, &rgba );

    }

    void GLViewport::_drawHUD()
    {
        TLRENDER_P();
        TLRENDER_GL();
        imaging::FontFamily fontFamily = imaging::FontFamily::NotoSans;
        uint16_t fontSize = 15;
        const imaging::Color4f labelColor(1.F, 1.F, 1.F);

        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics = p.fontSystem->getMetrics(fontInfo);
        auto lineHeight = fontMetrics.lineHeight;
        math::Vector2i pos( 20, 20 );

        const auto& player = p.timelinePlayers[0];
        const auto& path   = player->path();
        const auto& directory = path.getDirectory();
        const auto& name = path.getBaseName();
        int64_t    frame = player->currentTime().to_frames();
        const auto& num = path.getNumber();
        const auto& extension = path.getExtension();

        if ( is_valid_movie( extension.c_str() ) )
            frame = atoi( num.c_str() );

        char number[256]; number[0] = 0;
        if ( !num.empty() )
        {
            const uint8_t padding = path.getPadding();
            sprintf( number, "%0*" PRId64, padding, frame );
        }
        std::string fullname = name + number + extension;
        const auto& info   = player->timelinePlayer()->getIOInfo();
        const auto& video = info.video[0];
        const auto& video_name = video.name;
        char buf[256];

        if ( p.hud & HudDisplay::kDirectory )
            _drawText( p.fontSystem->getGlyphs(directory, fontInfo), pos,
                       lineHeight, labelColor );

        if ( p.hud & HudDisplay::kFilename )
            _drawText( p.fontSystem->getGlyphs(fullname, fontInfo), pos,
                       lineHeight, labelColor );

        if ( p.hud & HudDisplay::kResolution )
        {
            sprintf( buf, "%dx%d", video.size.w, video.size.h );
            _drawText( p.fontSystem->getGlyphs(buf, fontInfo), pos,
                       lineHeight, labelColor );
        }

        std::string tmp;
        if ( p.hud & HudDisplay::kFrame )
        {
            sprintf( buf, "F: %" PRId64 " ",
                     (int64_t)getTimelinePlayer()->currentTime().value() );
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kFPS )
        {
            sprintf( buf, "FPS: %.3f", p.ui->uiFPS->value() );
            tmp += buf;
        }
        if ( !tmp.empty() )
            _drawText( p.fontSystem->getGlyphs(tmp, fontInfo), pos,
                       lineHeight, labelColor );
    }
}
