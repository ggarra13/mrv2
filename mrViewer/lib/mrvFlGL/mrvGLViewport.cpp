// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.


#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlCore/Mesh.h>

#include <tlGlad/gl.h>

// mrViewer includes
#include <mrvFl/mrvIO.h>
#include <mrvFl/mrvTimelinePlayer.h>
#include <mrvFlGL/mrvGLViewport.h>
#include <mrvFlGL/mrvTimelineViewportInline.h>

#include <glm/gtc/matrix_transform.hpp>

// For main fltk event loop
#include <FL/Fl.H>

//! Define a variable, "p", that references the private implementation.
#define TLRENDER_GL()                           \
    auto& g = *_gl

namespace mrv
{
    using namespace tl;

    struct GLViewport::GLPrivate
    {
        std::weak_ptr<system::Context> context;

        // GL variables
        std::shared_ptr<timeline::IRender> render;
        std::shared_ptr<tl::gl::Shader> shader;
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
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
        const std::shared_ptr<system::Context>& context )
    {
        _gl->context = context;
    }

    void GLViewport::initializeGL()
    {
        TLRENDER_GL();
        try
        {
            gladLoaderLoadGL();
            if ( !g.render )
            {
                if (auto context = g.context.lock())
                {
                    g.render = gl::Render::create(context);
                }
            }

            if ( !g.shader )
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
            g.shader = gl::Shader::create(vertexSource, fragmentSource);
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = g.context.lock())
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

        if ( !valid() )
        {
            std::cerr << "valid GL" << std::endl;
            initializeGL();
            valid(1);
        }

        TLRENDER_P();
        TLRENDER_GL();

        const auto renderSize = _getRenderSize();
        try
        {
            if (renderSize.isValid())
            {
                std::cerr << "renderSize is valid" << std::endl;
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorMinifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.minify);
                    offscreenBufferOptions.colorMagnifyFilter = gl::getTextureFilter(p.displayOptions[0].imageFilters.magnify);
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(g.buffer, renderSize, offscreenBufferOptions))
                {
                    g.buffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                }
            }
            else
            {
                std::cerr << "renderSize is NOT valid" << std::endl;
                g.buffer.reset();
            }

            if (g.buffer)
            {
                gl::OffscreenBufferBinding binding(g.buffer);
                g.render->setColorConfig(p.colorConfig);
                g.render->begin(renderSize);
                g.render->drawVideo(
                    p.videoData,
                    timeline::tiles(p.compareOptions.mode,
                                    _getTimelineSizes()),
                    p.imageOptions,
                    p.displayOptions,
                    p.compareOptions);
                g.render->end();
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = g.context.lock())
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
        glClearColor(0.5F, 0.F, 0.F, 0.F);
        glClear(GL_COLOR_BUFFER_BIT);

        if (g.buffer)
        {
            g.shader->bind();
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
            auto mvp =math::Matrix4x4f(
                vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3] );

            g.shader->setUniform("transform.mvp", mvp);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g.buffer->getColorID());

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
            if (!g.vbo)
            {
                g.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
            }
            if (g.vbo)
            {
                g.vbo->copy(vboData);
            }

            if (!g.vao && g.vbo)
            {
                g.vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, g.vbo->getID());
            }
            if (g.vao && g.vbo)
            {
                g.vao->bind();
                g.vao->draw(GL_TRIANGLES, 0, g.vbo->getSize());
            }
        }

        TimelineViewport::draw();
    }


}
