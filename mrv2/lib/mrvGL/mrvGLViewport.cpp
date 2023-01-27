// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <cinttypes>

#include <locale.h>


#include <tlCore/FontSystem.h>
#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <Imath/ImathMatrix.h>

#include <tlGlad/gl.h>

// mrViewer includes
#include "mrViewer.h"

#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvColorSpaces.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvWidgets/mrvHorSlider.h" // @todo: refactor

#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLUtil.h"
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLViewport.h"
#ifdef USE_ONE_PIXEL_LINES
#   include "mrvGL/mrvGLOutline.h"
#endif

#include "mrvTools/mrvToolsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

#include <glm/gtc/matrix_transform.hpp>



// For main fltk event loop
#include <FL/Fl.H>

//! Define a variable, "gl", that references the private implementation.
#define TLRENDER_GL()                           \
    auto& gl = *_gl


namespace {
    const char* kModule = "view";
}

namespace mrv
{
    using namespace tl;


    struct Viewport::GLPrivate
    {
        std::weak_ptr<system::Context> context;

        // GL variables
        //! OpenGL Offscreen buffer
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer = nullptr;
        std::shared_ptr<tl::gl::Render> render = nullptr;
        std::shared_ptr<tl::gl::Shader> shader    = nullptr;
        int index = 0;
        int nextIndex = 1;
        GLuint pboIds[2];
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;

#ifdef USE_ONE_PIXEL_LINES
        tl::gl::Outline              outline;
#endif

    };


    Viewport::Viewport( int X, int Y, int W, int H, const char* L ) :
        TimelineViewport( X, Y, W, H, L ),
        _gl( new GLPrivate )
    {
        mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | FL_OPENGL3 );
    }

    Viewport::~Viewport()
    {
        TLRENDER_P();
        if ( p.image && p.rawImage )
        {
            free( p.image );
        }
    }

    void Viewport::setContext(
        const std::weak_ptr<system::Context>& context )
    {
        _gl->context = context;
    }

    void Viewport::_initializeGL()
    {
        TLRENDER_P();
        TLRENDER_GL();
        try
        {
            tl::gl::initGLAD();

            if ( !gl.render )
            {
                if (auto context = gl.context.lock())
                {
                    gl.render = gl::Render::create(context);
                }

                glGenBuffers( 2, gl.pboIds );
            }

            if ( !p.fontSystem )
            {
                if (auto context = gl.context.lock())
                {
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
                    "mat4 mvp;\n"
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
                try
                {
                    std::string latLongSource = readGLShader( "latLong.glsl" );
                    gl.shader = gl::Shader::create(latLongSource, fragmentSource);
                }
                catch ( const std::exception& e )
                {
                    std::cerr << e.what() << std::endl;
                }
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "mrv::Viewport",
                    e.what(),
                    log::Type::Error);
            }
        }
    }


    void Viewport::_drawCursor(const math::Matrix4x4f& mvp) const noexcept
    {
        TLRENDER_GL();
        TLRENDER_P();
        if ( p.actionMode != ActionMode::kScrub &&
             p.actionMode != ActionMode::kText &&
             p.actionMode != ActionMode::kSelection &&
             Fl::belowmouse() == this )
        {
            const imaging::Color4f color( 1.F, 1.F, 1.F, 1.F );
            std_any value;
            value = p.ui->app->settingsObject()->value( kPenSize );
            const float pen_size = std_any_cast<int>(value);
            p.mousePos = _getFocus();
            const auto& pos = _getRaster();
            drawCursor( gl.render, pos, pen_size, 2.0F, color, mvp );
        }
    }


    void
    Viewport::_drawRectangleOutline(
        const math::BBox2i& box,
        const imaging::Color4f& color,
        const math::Matrix4x4f& mvp ) const noexcept
    {
        TLRENDER_GL();
#if USE_ONE_PIXEL_LINES
        gl.outline.drawRect( box, color, mvp );
#else
        drawRectOutline( gl.render, box, color, 2.F, mvp );
#endif

    }


    void Viewport::draw()
    {
        TLRENDER_P();
        TLRENDER_GL();


        if ( !valid() )
        {
            _initializeGL();
            valid(1);
        }

#ifdef DEBUG_SPEED
        auto start_time = std::chrono::steady_clock::now();
#endif


        const auto& renderSize = getRenderSize();
        try
        {
            if (renderSize.isValid())
            {
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(gl.buffer, renderSize, offscreenBufferOptions))
                {
                    gl.buffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                    unsigned dataSize = renderSize.w * renderSize.h * 4
                                        * sizeof(GLfloat);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[0]);
                    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0,
                                 GL_STREAM_READ);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[1]);
                    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0,
                                 GL_STREAM_READ);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                }
            }
            else
            {
                gl.buffer.reset();
            }

            if (gl.buffer)
            {
                gl::OffscreenBufferBinding binding(gl.buffer);
                const char* oldloc= setlocale( LC_NUMERIC, "C" );
                gl.render->setColorConfig(p.colorConfigOptions);
                gl.render->setLUT(p.lutOptions);
                setlocale( LC_NUMERIC, oldloc );
                gl.render->begin(renderSize);
                gl.render->drawVideo(
                    p.videoData,
                    timeline::tiles(p.compareOptions.mode,
                                    _getTimelineSizes()),
                    p.imageOptions,
                    p.displayOptions,
                    p.compareOptions);
                if (p.masking > 0.0001F ) _drawCropMask( renderSize );
                gl.render->end();
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "Viewport",
                    e.what(),
                    log::Type::Error);
            }
        }

        const auto& viewportSize = getViewportSize();
        glViewport(
            0,
            0,
            GLsizei(viewportSize.w),
            GLsizei(viewportSize.h));

        float r, g, b, a = 1.0f;
        if ( !p.presentation )
        {
            uint8_t ur, ug, ub;
            Fl::get_color( p.ui->uiPrefs->uiPrefsViewBG->color(), ur, ug, ub );
            r = ur / 255.0f;
            g = ug / 255.0f;
            b = ub / 255.0f;
        }
        else
        {
            r = g = b = 0.0f;
        }

        glClearColor( r, g, b, a );
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

            if ( latLongTool )
            {
                const auto* t = latLongTool;
                auto textureSize = math::Vector2f( renderSize.w, renderSize.h );
                gl.shader->setUniform("vTextureSize", textureSize);
                float v;
                v = t->hAperture->value();
                std::cerr << "hApeture=" << v << std::endl;
                gl.shader->setUniform("hAperture", v );
                v = t->vAperture->value();
                std::cerr << "vApeture=" << v << std::endl;
                gl.shader->setUniform("vAperture", v );
                v = t->focalLength->value();
                std::cerr << "focalLength=" << v << std::endl;
                gl.shader->setUniform("focalLength", v);
                v = t->rotateX->value();
                std::cerr << "rotateX=" << v << std::endl;
                gl.shader->setUniform("rotateX", v );
                v = t->rotateY->value();
                std::cerr << "rotateY=" << v << std::endl;
                gl.shader->setUniform("rotateY", v );
            }
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gl.buffer->getColorID());

            const auto mesh = geom::bbox(math::BBox2i(0, 0, renderSize.w, renderSize.h));
            if (!gl.vbo)
            {
                gl.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
            }
            if (gl.vbo)
            {
                gl.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
            }

            if (!gl.vao && gl.vbo)
            {
                gl.vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, gl.vbo->getID());
            }
            if (gl.vao && gl.vbo)
            {
                gl.vao->bind();
                gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());

                math::BBox2i selection = p.colorAreaInfo.box = p.selection;
                if ( selection.min != selection.max )
                {
                    // Check min < max
                    if ( selection.min.x > selection.max.x )
                    {
                        float tmp = selection.max.x;
                        selection.max.x = selection.min.x;
                        selection.min.x = tmp;
                    }
                    if ( selection.min.y > selection.max.y )
                    {
                        float tmp = selection.max.y;
                        selection.max.y = selection.min.y;
                        selection.min.y = tmp;
                    }
                    // Copy it again in case it changed
                    p.colorAreaInfo.box = selection;

                    _mapBuffer();
                }
                else
                {
                    p.image = nullptr;
                }
                if ( colorAreaTool )
                {
                    _calculateColorArea( p.colorAreaInfo );
                    colorAreaTool->update( p.colorAreaInfo );
                }
                if ( histogramTool )
                {
                    histogramTool->update( p.colorAreaInfo );
                }
                if ( vectorscopeTool )
                {
                    vectorscopeTool->update( p.colorAreaInfo );
                }

                // Uodate the pixel bar from here only if we are playing a movie
                // and one that is not 1 frames long.
                bool update = ! _shouldUpdatePixelBar();
                if ( update ) updatePixelBar();

                _unmapBuffer();

                Fl_Color c = p.ui->uiPrefs->uiPrefsViewSelection->color();
                uint8_t r, g, b;
                Fl::get_color( c, r, g, b );

                const imaging::Color4f color(r / 255.F, g / 255.F,
                                             b / 255.F);

                if ( p.selection.min != p.selection.max )
                {
                    _drawRectangleOutline( p.selection, color, mvp );
                }

                if ( p.showAnnotations ) _drawAnnotations(mvp);
                if ( p.safeAreas ) _drawSafeAreas();

                _drawCursor(mvp);

            }

            if ( p.hudActive && p.hud != HudDisplay::kNone ) _drawHUD();
        }


        MultilineInput* w = getMultilineInput();
        if ( w )
        {
            std_any value;
            value = p.ui->app->settingsObject()->value( kFontSize );
            int font_size = std_any_cast<int>( value );
            double pixels_unit = pixels_per_unit();
            double pct = viewportSize.h / 1024.F;
            double fontSize = font_size * pct * p.viewZoom;
            w->textsize( fontSize );
            math::Vector2i pos( w->pos.x, w->pos.y );
            // This works to pan without a change in zoom!
            // pos.x = pos.x + p.viewPos.x / p.viewZoom
            //         - w->viewPos.x / w->viewZoom;
            // pos.y = pos.y - p.viewPos.y / p.viewZoom
            //         - w->viewPos.y / w->viewZoom;
            // cur.x = (cur.x - p.viewPos.x / pixels_unit) / p.viewZoom;
            // cur.y = (cur.y - p.viewPos.y / pixels_unit) / p.viewZoom;

            // pos.x = (pos.x - w->viewPos.x / pixels_unit) /
            //         w->viewZoom;
            // pos.y = (pos.y - w->viewPos.y / pixels_unit) /
            //         w->viewZoom;

            // pos.x += cur.x;
            // pos.y -= cur.y;

            // std::cerr << "pos=" << pos << std::endl;
            // std::cerr << "p.viewPos=" << p.viewPos << std::endl;
            // std::cerr << "END " << pos << std::endl;
            w->Fl_Widget::position( pos.x, pos.y );
        }

#ifdef USE_OPENGL2
        Fl_Gl_Window::draw_begin(); // Set up 1:1 projectionç
        Fl_Window::draw();          // Draw FLTK children
        glViewport(0, 0, viewportSize.w, viewportSize.h);
        if ( p.showAnnotations ) _drawAnnotationsGL2();
        Fl_Gl_Window::draw_end();   // Restore GL state
#else
        Fl_Gl_Window::draw();
#endif

#ifdef DEBUG_SPEED
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end_time - start_time;
        std::cout << "GL::draw() duration " << diff.count() << std::endl;
#endif
    }

#ifdef USE_OPENGL2

    void Viewport::_drawAnnotationsGL2()
    {
        TLRENDER_P();
        TLRENDER_GL();

        const auto& player = getTimelinePlayer();
        if (!player) return;

        const otime::RationalTime& time = p.videoData[0].time;
        int64_t frame = time.to_frames();

        const auto& annotations = player->getAnnotations( p.ghostPrevious,
                                                          p.ghostNext );
        if ( !annotations.empty() )
        {
            glStencilMask(~0);
            glClear(GL_STENCIL_BUFFER_BIT);

            float pixel_unit = pixels_per_unit();
            const auto& viewportSize = getViewportSize();
            const auto& renderSize   = getRenderSize();

            for ( const auto& annotation : annotations )
            {
                int64_t annotationFrame = annotation->frame();
                float alphamult = 0.F;
                if ( frame == annotationFrame || annotation->allFrames() ) alphamult = 1.F;
                else
                {
                    if ( p.ghostPrevious )
                    {
                        for ( short i = p.ghostPrevious-1; i > 0; --i )
                        {
                            if ( frame - i == annotationFrame )
                            {
                                alphamult = 1.F - (float)i/p.ghostPrevious;
                                break;
                            }
                        }
                    }
                    if ( p.ghostNext )
                    {
                        for ( short i = 1; i < p.ghostNext; ++i )
                        {
                            if ( frame + i == annotationFrame )
                            {
                                alphamult = 1.F - (float)i/p.ghostNext;
                                break;
                            }
                        }
                    }
                }

                if ( alphamult == 0.F ) continue;

                const auto& shapes = annotation->shapes();
                math::Vector2i pos;

                pos.x = p.viewPos.x;
                pos.y = p.viewPos.y;
                pos.x /= pixel_unit;
                pos.y /= pixel_unit;
                glm::mat4x4 vm(1.F);
                vm = glm::translate(vm, glm::vec3(pos.x, pos.y, 0.F));
                vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));

                // No projection matrix.  Thar's set by FLTK ( and we
                // reset it -- flip it in Y -- inside mrvGL2TextShape.cpp ).
                auto mvp = math::Matrix4x4f(
                    vm[0][0], vm[0][1], vm[0][2], vm[0][3],
                    vm[1][0], vm[1][1], vm[1][2], vm[1][3],
                    vm[2][0], vm[2][1], vm[2][2], vm[2][3],
                    vm[3][0], vm[3][1], vm[3][2], vm[3][3] );

                for ( auto& shape : shapes )
                {
                    auto textShape =
                        dynamic_cast< GL2TextShape* >( shape.get() );
                    if ( !textShape ) continue;

                    float a = shape->color.a;
                    shape->color.a *= alphamult;
                    textShape->pixels_per_unit = pixels_per_unit();
                    textShape->w    = w();
                    textShape->h    = h();
                    textShape->viewZoom = p.viewZoom;
                    shape->matrix = mvp;
                    shape->draw( gl.render );
                    shape->color.a = a;
                }
            }
        }
    }
#endif

    void Viewport::_drawAnnotations(math::Matrix4x4f& mvp)
    {
        TLRENDER_P();
        TLRENDER_GL();

        const auto& player = getTimelinePlayer();
        if (!player) return;

        const otime::RationalTime& time = p.videoData[0].time;
        int64_t frame = time.to_frames();

        const auto& annotations = player->getAnnotations( p.ghostPrevious,
                                                          p.ghostNext );
        if ( !annotations.empty() )
        {
            glStencilMask(~0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable( GL_STENCIL_TEST );

            const auto& viewportSize = getViewportSize();
            const auto& renderSize = getRenderSize();

            for ( const auto& annotation : annotations )
            {
                int64_t annotationFrame = annotation->frame();
                float alphamult = 0.F;
                if ( frame == annotationFrame || annotation->allFrames() ) alphamult = 1.F;
                else
                {
                    if ( p.ghostPrevious  )
                    {
                        for ( short i = p.ghostPrevious-1; i > 0; --i )
                        {
                            if ( frame - i == annotationFrame )
                            {
                                alphamult = 1.F - (float)i/p.ghostPrevious;
                                break;
                            }
                        }
                    }
                    if ( p.ghostNext )
                    {
                        for ( short i = 1; i < p.ghostNext; ++i )
                        {
                            if ( frame + i == annotationFrame )
                            {
                                alphamult = 1.F - (float)i/p.ghostNext;
                                break;
                            }
                        }
                    }
                }

                if ( alphamult == 0.F ) continue;

                // Shapes are drawn in reverse order, so the erase path works
                const auto& shapes = annotation->shapes();
                ShapeList::const_reverse_iterator i = shapes.rbegin();
                ShapeList::const_reverse_iterator e = shapes.rend();

                for ( ; i != e; ++i )
                {
                    const auto& shape = *i;
#ifdef USE_OPENGL2
                    auto gl2Shape = dynamic_cast< GL2TextShape* >( shape.get() );
                    if ( gl2Shape ) continue;
#else
                    auto textShape = dynamic_cast< GLTextShape* >( shape.get() );
                    if ( textShape && !textShape->text.empty() )
                    {
                        glm::mat4x4 vm(1.F);
                        vm = glm::translate(vm, glm::vec3(p.viewPos.x,
                                                          p.viewPos.y, 0.F));
                        vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));
                        glm::mat4x4 pm = glm::ortho(
                            0.F,
                            static_cast<float>(viewportSize.w),
                            0.F,
                            static_cast<float>(viewportSize.h),
                            -1.F,
                            1.F);
                        glm::mat4x4 vpm = pm * vm;
                        vpm = glm::scale(vpm, glm::vec3(1.F, -1.F, 1.F));
                        mvp = math::Matrix4x4f(
                            vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                            vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                            vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                            vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3] );
                    }
#endif
                    float a = shape->color.a;
                    shape->color.a *= alphamult;
                    shape->matrix = mvp;
                    shape->draw( gl.render );
                    shape->color.a = a;
                }
            }
            glDisable( GL_STENCIL_TEST );
        }
    }


    void Viewport::_drawCropMask(
        const imaging::Size& renderSize ) const noexcept
    {
        TLRENDER_GL();

        double aspectY = (double) renderSize.w / (double) renderSize.h;
        double aspectX = (double) renderSize.h / (double) renderSize.w;

        double target_aspect = 1.0 / _p->masking;
        double amountY = (0.5 - target_aspect * aspectY / 2);
        double amountX = (0.5 - _p->masking * aspectX / 2);

        bool vertical = true;
        if ( amountY < amountX )
        {
            vertical = false;
        }

        imaging::Color4f maskColor( 0, 0, 0, 1 );

        if ( vertical )
        {
            int Y = renderSize.h * amountY;
            math::BBox2i box( 0, 0, renderSize.w, Y );
            gl.render->drawRect( box, maskColor );
            box.max.y = renderSize.h;
            box.min.y = renderSize.h - Y;
            gl.render->drawRect( box, maskColor );
        }
        else
        {
            int X = renderSize.w * amountX;
            math::BBox2i box( 0, 0, X, renderSize.h );
            gl.render->drawRect( box, maskColor );
            box.max.x = renderSize.w;
            box.min.x = renderSize.w - X;
            gl.render->drawRect( box, maskColor );
        }

    }

    inline
    void Viewport::_drawText(
        const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
        math::Vector2i& pos, const int16_t lineHeight,
        const imaging::Color4f& labelColor) const noexcept
    {
        TLRENDER_GL();
        const imaging::Color4f shadowColor(0.F, 0.F, 0.F, 0.7F);
        math::Vector2i shadowPos{ pos.x + 2, pos.y + 2 };
        gl.render->drawText( glyphs, shadowPos, shadowColor );
        gl.render->drawText( glyphs, pos, labelColor );
        pos.y += lineHeight;
    }

    void Viewport::_calculateColorAreaFullValues( area::Info& info ) noexcept
    {
        TLRENDER_P();
        TLRENDER_GL();

        PixelToolBarClass* c = p.ui->uiPixelWindow;
        BrightnessType brightness_type =(BrightnessType) c->uiLType->value();
        int hsv_colorspace = c->uiBColorType->value() + 1;

        int maxX = info.box.max.x;
        int maxY = info.box.max.y;
        const auto& renderSize = gl.buffer->getSize();

        for ( int Y = info.box.y(); Y < maxY; ++Y )
        {
            for ( int X = info.box.x(); X < maxX; ++X )
            {
                imaging::Color4f rgba, hsv;
                rgba.b = p.image[ ( X + Y * renderSize.w ) * 4 ];
                rgba.g = p.image[ ( X + Y * renderSize.w ) * 4 + 1 ];
                rgba.r = p.image[ ( X + Y * renderSize.w ) * 4 + 2 ];
                rgba.a = p.image[ ( X + Y * renderSize.w ) * 4 + 3 ];


                info.rgba.mean.r += rgba.r;
                info.rgba.mean.g += rgba.g;
                info.rgba.mean.b += rgba.b;
                info.rgba.mean.a += rgba.a;

                if ( rgba.r < info.rgba.min.r ) info.rgba.min.r = rgba.r;
                if ( rgba.g < info.rgba.min.g ) info.rgba.min.g = rgba.g;
                if ( rgba.b < info.rgba.min.b ) info.rgba.min.b = rgba.b;
                if ( rgba.a < info.rgba.min.a ) info.rgba.min.a = rgba.a;

                if ( rgba.r > info.rgba.max.r ) info.rgba.max.r = rgba.r;
                if ( rgba.g > info.rgba.max.g ) info.rgba.max.g = rgba.g;
                if ( rgba.b > info.rgba.max.b ) info.rgba.max.b = rgba.b;
                if ( rgba.a > info.rgba.max.a ) info.rgba.max.a = rgba.a;

                hsv = rgba_to_hsv( hsv_colorspace, rgba );
                hsv.a = calculate_brightness( rgba, brightness_type );
                hsv_to_info( hsv, info );
            }
        }

        int num = info.box.w() * info.box.h();
        info.rgba.mean.r /= num;
        info.rgba.mean.g /= num;
        info.rgba.mean.b /= num;
        info.rgba.mean.a /= num;

        info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
        info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
        info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
        info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

        info.hsv.mean.r /= num;
        info.hsv.mean.g /= num;
        info.hsv.mean.b /= num;
        info.hsv.mean.a /= num;

        info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
        info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
        info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
        info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
    }

    void Viewport::_calculateColorArea( area::Info& info )
    {
        TLRENDER_P();
        TLRENDER_GL();

        if( !p.image ) return;

        info.rgba.max.r = std::numeric_limits<float>::min();
        info.rgba.max.g = std::numeric_limits<float>::min();
        info.rgba.max.b = std::numeric_limits<float>::min();
        info.rgba.max.a = std::numeric_limits<float>::min();

        info.rgba.min.r = std::numeric_limits<float>::max();
        info.rgba.min.g = std::numeric_limits<float>::max();
        info.rgba.min.b = std::numeric_limits<float>::max();
        info.rgba.min.a = std::numeric_limits<float>::max();

        info.rgba.mean.r = info.rgba.mean.g = info.rgba.mean.b =
        info.rgba.mean.a = 0.F;


        info.hsv.max.r = std::numeric_limits<float>::min();
        info.hsv.max.g = std::numeric_limits<float>::min();
        info.hsv.max.b = std::numeric_limits<float>::min();
        info.hsv.max.a = std::numeric_limits<float>::min();

        info.hsv.min.r = std::numeric_limits<float>::max();
        info.hsv.min.g = std::numeric_limits<float>::max();
        info.hsv.min.b = std::numeric_limits<float>::max();
        info.hsv.min.a = std::numeric_limits<float>::max();

        info.hsv.mean.r = info.hsv.mean.g = info.hsv.mean.b =
        info.hsv.mean.a = 0.F;

        if ( p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull )
            _calculateColorAreaFullValues( info );
        else
            _calculateColorAreaRawValues( info );
    }


    void Viewport::_mapBuffer() const noexcept
    {
        TLRENDER_GL();
        TLRENDER_P();

        if ( p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull )
        {

            // For faster access, we muse use BGRA.
            constexpr GLenum format = GL_BGRA;
            constexpr GLenum type = GL_FLOAT;

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE );

            gl::OffscreenBufferBinding binding(gl.buffer);
            const imaging::Size& renderSize = gl.buffer->getSize();

            if ( p.rawImage ) free( p.image );

            bool update = _shouldUpdatePixelBar();

            // set the target framebuffer to read
            // "index" is used to read pixels from framebuffer to a PBO
            // "nextIndex" is used to update pixels in the other PBO
            gl.index = (gl.index + 1) % 2;
            gl.nextIndex = (gl.index + 1) % 2;

            // Set the target framebuffer to read
            glReadBuffer(GL_FRONT);

            // read pixels from framebuffer to PBO
            // glReadPixels() should return immediately.
            glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);
            glReadPixels(0, 0, renderSize.w, renderSize.h, format, type, 0);

            // map the PBO to process its data by CPU
            glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.nextIndex]);

            // We are stopped, read the first PBO.
            if ( update )
                glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);

            p.image = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            p.rawImage = false;
        }
        else
        {
            TimelineViewport::_mapBuffer();
        }
    }


    void Viewport::_unmapBuffer() const noexcept
    {
        TLRENDER_P();

        if ( p.image )
        {
            if ( !p.rawImage )
            {
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                p.image = nullptr;
                p.rawImage = true;
            }
        }

        // back to conventional pixel operation
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }


    void Viewport::_readPixel( imaging::Color4f& rgba ) const noexcept
    {
        // If window was not yet mapped, return immediately
        if ( !valid() ) return;

        TLRENDER_P();
        TLRENDER_GL();

        math::Vector2i pos;
        pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
        pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;

        if ( p.ui->uiPixelWindow->uiPixelValue->value() != PixelValue::kFull )
        {

            rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

            for ( const auto& video : p.videoData )
            {
                for ( const auto& layer : video.layers )
                {
                    const auto& image = layer.image;
                    if ( ! image->isValid() ) continue;

                    imaging::Color4f pixel, pixelB;

                    _getPixelValue( pixel, image, pos );


                    const auto& imageB = layer.image;
                    if ( imageB->isValid() )
                    {
                        _getPixelValue( pixelB, imageB, pos );

                        if ( layer.transition ==
                             timeline::Transition::Dissolve )
                        {
                            float f2 = layer.transitionValue;
                            float  f = 1.0 - f2;
                            pixel.r = pixel.r * f + pixelB.r * f2;
                            pixel.g = pixel.g * f + pixelB.g * f2;
                            pixel.b = pixel.b * f + pixelB.b * f2;
                            pixel.a = pixel.a * f + pixelB.a * f2;
                        }
                    }
                    rgba.r += pixel.r;
                    rgba.g += pixel.g;
                    rgba.b += pixel.b;
                    rgba.a += pixel.a;
                }
            }
        }
        else
        {
            // This is needed as the FL_MOVE of fltk wouuld get called
            // before the draw routine
            if ( !gl.buffer ) return;

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE );

            gl::OffscreenBufferBinding binding(gl.buffer);

            constexpr GLenum type = GL_FLOAT;

            // We use ReadPixels when the movie is stopped or has only a
            // a single frame.
            bool update = _shouldUpdatePixelBar();
            if ( update )
            {
                glReadPixels( pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                return;
            }

            if ( !p.image ) _mapBuffer();

            if( p.image )
            {
                const imaging::Size& renderSize = gl.buffer->getSize();
                rgba.b = p.image[ ( pos.x + pos.y * renderSize.w ) * 4 ];
                rgba.g = p.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 1 ];
                rgba.r = p.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 2 ];
                rgba.a = p.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 3 ];
            }
        }

        _unmapBuffer();
    }


    void Viewport::_drawSafeAreas(
        const float percentX, const float percentY,
        const float pixelAspectRatio,
        const imaging::Color4f& color,
        const math::Matrix4x4f& mvp,
        const char* label ) const noexcept
    {
        TLRENDER_GL();
        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();
        double aspectX = (double) renderSize.h / (double) renderSize.w;
        double aspectY = (double) renderSize.w / (double) renderSize.h;

        double amountY = (0.5 - percentY * aspectY / 2);
        double amountX = (0.5 - percentX * aspectX / 2);

        bool vertical = true;
        if ( amountY < amountX )
        {
            vertical = false;
        }

        math::BBox2i box;
        int X, Y;
        if ( vertical )
        {
            X = renderSize.w * percentX;
            Y = renderSize.h * amountY;
        }
        else
        {
          X = renderSize.w * amountX / pixelAspectRatio;
          Y = renderSize.h * percentY;
        }
        box.min.x = renderSize.w - X;
        box.min.y = -(renderSize.h - Y);
        box.max.x = X;
        box.max.y = -Y;
#if 0
        // @bug:
        //
        // Using USE_ONE_PIXEL_LINES would make the primary display flicker
        // after the secondary one was closed.
        _drawRectangleOutline( box, color, mvp );
#else
        int width = 2 * renderSize.w / viewportSize.w;
        if ( width < 2 ) width = 2;
        drawRectOutline( gl.render, box, color, width, mvp );
#endif

        //
        // Draw the text too
        //
        static const std::string fontFamily = "NotoSans-Regular";
        Viewport* self = const_cast< Viewport* >( this );
        const imaging::FontInfo fontInfo(fontFamily, 12 * width);
        const auto glyphs = _p->fontSystem->getGlyphs( label, fontInfo );
        math::Vector2i pos( box.max.x, box.max.y-2*width );
        // Set the projection matrix
        gl.render->setMatrix( mvp );
        gl.render->drawText( glyphs, pos, color );
    }

    void Viewport::_drawSafeAreas() const noexcept
    {
        TLRENDER_P();
        if ( p.timelinePlayers.empty() ) return;
        const auto& player = p.timelinePlayers[0];
        const auto& info   = player->timelinePlayer()->getIOInfo();
        const auto& video  = info.video[0];
        const auto pr      = video.size.pixelAspectRatio;

        const auto& viewportSize = getViewportSize();
        const auto& renderSize = getRenderSize();

        glm::mat4x4 vm(1.F);
        vm = glm::translate(vm, glm::vec3(p.viewPos.x,
                                          p.viewPos.y, 0.F));
        vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));
        glm::mat4x4 pm = glm::ortho(
            0.F,
            static_cast<float>(viewportSize.w),
            0.F,
            static_cast<float>(viewportSize.h),
            -1.F,
            1.F);
        glm::mat4x4 vpm = pm * vm;
        vpm = glm::scale(vpm, glm::vec3(1.F, -1.F, 1.F));
        auto mvp = math::Matrix4x4f(
            vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
            vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
            vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
            vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3] );

        double aspect = (double) renderSize.w / pr / (double) renderSize.h;
        if ( aspect <= 1.78 )
        {
            // For HDTV, NTSC or PAL, we just use the action/title areas
            imaging::Color4f color( 1.F, 0.F, 0.F );
            _drawSafeAreas( aspect * 0.9, 0.9F, pr, color, mvp, "tv action" );
            _drawSafeAreas( aspect * 0.8F, 0.8F, pr, color, mvp, "tv title" );
        }
        else
        {
            // For film, we use the different film ratios
            imaging::Color4f color( 1.F, 0.F, 0.F );
            // Assume film, draw 2.35, 1.85, 1.66 and hdtv areas
            _drawSafeAreas( 2.35, 1.F, pr, color, mvp, _("2.35") );

            // Draw HDTV safe aeas
            _drawSafeAreas( 1.77 * 0.9, 0.9F, pr, color, mvp, "tv action" );
            _drawSafeAreas( 1.77 * 0.8F, 0.8F, pr, color, mvp, "tv title" );

            color = imaging::Color4f( 1.F, 1.0f, 0.F );
            _drawSafeAreas( 1.89, 1.F, pr, color, mvp, _("1.85") );
            color = imaging::Color4f( 0.F, 1.0f, 1.F );
            _drawSafeAreas( 1.66, 1.F, pr, color, mvp, _("1.66") );
            // Draw hdtv too
            color = imaging::Color4f( 1.F, 0.0f, 1.F );
            _drawSafeAreas( 1.77, 1.0, pr, color, mvp, N_("hdtv") );
        }
    }

    int Viewport::handle( int event )
    {
        TLRENDER_GL();
        TLRENDER_P();
        int ok = TimelineViewport::handle( event );
        if ( event == FL_HIDE )
        {
            if ( gl.render )
                glDeleteBuffers( 2, gl.pboIds );
            gl.render.reset();
            gl.buffer.reset();
            gl.shader.reset();
            gl.vbo.reset();
            gl.vao.reset();
            p.fontSystem.reset();
            gl.index = 0;
            gl.nextIndex = 1;
            valid(0);
            context_valid(0);
            return 1;
        }
        return ok;
    }

    void Viewport::_drawHUD() const noexcept
    {
        TLRENDER_P();
        TLRENDER_GL();

        Viewport* self = const_cast< Viewport* >( this );

        const auto& viewportSize = getViewportSize();

        timeline::RenderOptions renderOptions;
        renderOptions.clear = false;
        gl.render->begin( viewportSize, renderOptions );

        static const std::string fontFamily = "NotoSans-Regular";
        uint16_t fontSize = 12 * self->pixels_per_unit();

        Fl_Color c = p.ui->uiPrefs->uiPrefsViewHud->color();
        uint8_t r, g, b;
        Fl::get_color( c, r, g, b );

        const imaging::Color4f labelColor(r / 255.F, g / 255.F, b / 255.F);

        char buf[512];
        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics =
            p.fontSystem->getMetrics(fontInfo);
        auto lineHeight = fontMetrics.lineHeight;
        math::Vector2i pos( 20, lineHeight*2 );

        const auto& player = p.timelinePlayers[0];
        if ( !player ) return;

        const auto& path   = player->path();
        const otime::RationalTime& time = p.videoData[0].time;
        int64_t frame = time.to_frames();


        if ( p.hud & HudDisplay::kDirectory )
        {
            const auto& directory = path.getDirectory();
            _drawText( p.fontSystem->getGlyphs(directory, fontInfo),
                       pos, lineHeight, labelColor );
        }

        if ( p.hud & HudDisplay::kFilename )
        {
            const std::string& fullname =
                createStringFromPathAndTime( path, time );
            _drawText( p.fontSystem->getGlyphs(fullname, fontInfo), pos,
                       lineHeight, labelColor );
        }

        if ( p.hud & HudDisplay::kResolution )
        {
            const auto& info   = player->timelinePlayer()->getIOInfo();
            const auto& video = info.video[0];
            if ( video.size.pixelAspectRatio != 1.0 )
            {
                int width = video.size.w * video.size.pixelAspectRatio;
                snprintf( buf, 512, "%d x %d  ( %.3g )  %d x %d",
                          video.size.w, video.size.h,
                          video.size.pixelAspectRatio, width, video.size.h);
            }
            else
            {
                snprintf( buf, 512, "%d x %d", video.size.w, video.size.h );
            }
            _drawText( p.fontSystem->getGlyphs(buf, fontInfo), pos,
                       lineHeight, labelColor );
        }


        std::string tmp;
        tmp.reserve( 512 );
        if ( p.hud & HudDisplay::kFrame )
        {
            snprintf( buf, 512, "F: %" PRId64 " ",frame);
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kFrameRange )
        {
            const auto& range = player->timeRange();
            frame = range.start_time().to_frames();
            const int64_t last_frame = range.end_time_inclusive().to_frames();
            snprintf( buf, 512, "Range: %" PRId64 " -  %" PRId64,
                      frame, last_frame );
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kTimecode )
        {
            snprintf( buf, 512, "TC: %s ", time.to_timecode(nullptr).c_str() );
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kFPS )
        {

            auto time_diff = ( time - p.lastTime );
            int64_t frame_diff = time_diff.to_frames();
            int64_t absdiff = std::abs(frame_diff);
            if ( absdiff > 1 && absdiff < 10 )
            {
                p.skippedFrames += absdiff - 1;
            }
            snprintf( buf, 512, "SF: %" PRIu64 " FPS: %.3f", p.skippedFrames,
                      player->speed() );
            tmp += buf;
        }

        p.lastTime = time;

        if ( !tmp.empty() )
            _drawText( p.fontSystem->getGlyphs(tmp, fontInfo), pos,
                       lineHeight, labelColor );

        tmp.clear();
        if ( p.hud & HudDisplay::kFrameCount )
        {
            const otime::TimeRange&    range = player->timeRange();
            const otime::RationalTime& duration = range.end_time_inclusive() -
                                                  range.start_time();
            snprintf( buf, 512, "FC: %" PRId64, (int64_t)duration.to_frames() );
            tmp += buf;
        }


        if ( !tmp.empty() )
            _drawText( p.fontSystem->getGlyphs(tmp, fontInfo), pos,
                       lineHeight, labelColor );

        if ( p.hud & HudDisplay::kAttributes )
        {
            const auto& info   = player->timelinePlayer()->getIOInfo();
            for ( const auto& tag : info.tags )
            {
                if ( pos.y > viewportSize.h ) return;
                snprintf( buf, 512, "%s = %s",
                          tag.first.c_str(), tag.second.c_str() );
                _drawText( p.fontSystem->getGlyphs(buf, fontInfo), pos,
                           lineHeight, labelColor );
            }
        }

    }
}
