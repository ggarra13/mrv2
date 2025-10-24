// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLLines.h"

#include "mrvVoice/mrvVoiceOver.h"

#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

#include <tlDraw/Shape.h>

#include <FL/Enumerations.H>

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>

namespace mrv
{
    using namespace tl;

    class GLShape : public draw::Shape
    {
    public:
        GLShape() :
            draw::Shape()
        {
        }
        virtual ~GLShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) = 0;
    };

    class GLCircleShape : public GLShape
    {
    public:
        GLCircleShape() :
            GLShape(),
            radius(1.0) {};
        virtual ~GLCircleShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

        math::Vector2f center;
        double radius;
        opengl::Lines lines;
    };

    void to_json(nlohmann::json& json, const GLCircleShape& value);
    void from_json(const nlohmann::json& json, GLCircleShape& value);

    class GLFilledCircleShape : public GLCircleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLFilledCircleShape& value);
    void from_json(const nlohmann::json& json, GLFilledCircleShape& value);

    class GLPathShape : public draw::PathShape
    {
    public:
        GLPathShape() :
            draw::PathShape() {};
        virtual ~GLPathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&);
        opengl::Lines lines;
    };

    void to_json(nlohmann::json& json, const GLPathShape& value);
    void from_json(const nlohmann::json& json, GLPathShape& value);

    class GLPolygonShape : public GLPathShape
    {
    public:
        GLPolygonShape() :
            GLPathShape() {};
        virtual ~GLPolygonShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

        opengl::Lines lines;
    };

    class GLFilledPolygonShape : public GLPolygonShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLFilledPolygonShape& value);
    void from_json(const nlohmann::json& json, GLFilledPolygonShape& value);

    class GLArrowShape : public GLPathShape
    {
    public:
        GLArrowShape() :
            GLPathShape() {};
        virtual ~GLArrowShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLArrowShape& value);
    void from_json(const nlohmann::json& json, GLArrowShape& value);

    class GLRectangleShape : public GLPathShape
    {
    public:
        GLRectangleShape() :
            GLPathShape() {};
        virtual ~GLRectangleShape() {};
        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLRectangleShape& value);
    void from_json(const nlohmann::json& json, GLRectangleShape& value);

    class GLFilledRectangleShape : public GLRectangleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLFilledRectangleShape& value);
    void from_json(const nlohmann::json& json, GLFilledRectangleShape& value);

    class GLTextShape : public GLPathShape
    {
    public:
        GLTextShape(const std::shared_ptr<image::FontSystem> f) :
            GLPathShape(),
            fontSize(30),
            fontSystem(f) {};
        virtual ~GLTextShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

    public:
        std::string fontFamily = "NotoSans-Regular";
        std::string txt;
        std::string text;
        uint16_t fontSize;
        Fl_Font font;
        float viewZoom = 1.F;
        std::shared_ptr<image::FontSystem> fontSystem;
    };

    void to_json(nlohmann::json& json, const GLTextShape& value);
    void from_json(const nlohmann::json& json, GLTextShape& value);

#ifdef USE_OPENGL2
    class GL2TextShape : public GLPathShape
    {
    public:
        GL2TextShape() :
            GLPathShape() {};
        virtual ~GL2TextShape() {};

        //! Auxiliary function to set the raster coordinates with no clipping
        bool setRasterPos(double x, double y, size_t textLength);

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

    public:
        std::string txt; // Copy of text as we are processing it.
        std::string text;
        Fl_Font font;
        float fontSize = 30;
        int w;
        int h;
        double pixels_per_unit = 1.F;
        double viewZoom = 1.F;
    };

    void to_json(nlohmann::json& json, const GL2TextShape& value);
    void from_json(const nlohmann::json& json, GL2TextShape& value);

#endif

    class GLLinkShape : public GLPathShape
    {
   public:
        GLLinkShape() :
            GLPathShape() {};
        virtual ~GLLinkShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

        void open();
        bool edit();
        int handle(int event);
        
    public:
        std::string url;
        std::string title;
    };

    void to_json(nlohmann::json& json, const GLLinkShape& value);
    void from_json(const nlohmann::json& json, GLLinkShape& value);

    
    class GLErasePathShape : public GLPathShape
    {
    public:
        GLErasePathShape() :
            GLPathShape() {};
        virtual ~GLErasePathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

        bool rectangle = false;
        bool drawing = true;
        float mult = 1.F;
    };

    void to_json(nlohmann::json& json, const GLErasePathShape& value);
    void from_json(const nlohmann::json& json, GLErasePathShape& value);

    typedef std::vector< std::shared_ptr< draw::Shape > > ShapeList;

    
    /** 
     * Auxiliary class used to draw a voice over icon.
     * 
     */
    class GLVoiceOverShape
    {
    public:
        GLVoiceOverShape()
            {
            };
        ~GLVoiceOverShape() {};
        
        void draw(const std::shared_ptr<timeline::IRender>&,
                  const voice::MouseData& mouse);
        
        voice::RecordStatus status;
        math::Vector2f center;
        float mult = 1.F;
    };


} // namespace mrv
