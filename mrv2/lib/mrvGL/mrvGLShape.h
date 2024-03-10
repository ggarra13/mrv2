// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <limits>
#include <cmath>
#include <vector>
#include <iostream>

#include <FL/Enumerations.H>

#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

#include "mrvDraw/Shape.h"

#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLLines.h"

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
        virtual ~GLShape(){};

        virtual void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) = 0;
    };

    class GLCircleShape : public GLShape
    {
    public:
        GLCircleShape() :
            GLShape(),
            radius(1.0){};
        virtual ~GLCircleShape(){};

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

        math::Vector2f center;
        double radius;
        opengl::Lines lines;
    };

    void to_json(nlohmann::json& json, const GLCircleShape& value);
    void from_json(const nlohmann::json& json, GLCircleShape& value);

    class GLPathShape : public draw::PathShape
    {
    public:
        GLPathShape() :
            draw::PathShape(){};
        virtual ~GLPathShape(){};

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
        opengl::Lines lines;
    };

    void to_json(nlohmann::json& json, const GLPathShape& value);
    void from_json(const nlohmann::json& json, GLPathShape& value);

    class GLArrowShape : public GLPathShape
    {
    public:
        GLArrowShape() :
            GLPathShape(){};
        virtual ~GLArrowShape(){};

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLArrowShape& value);
    void from_json(const nlohmann::json& json, GLArrowShape& value);

    class GLRectangleShape : public GLPathShape
    {
    public:
        GLRectangleShape() :
            GLPathShape(){};
        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
        virtual ~GLRectangleShape(){};
    };

    void to_json(nlohmann::json& json, const GLRectangleShape& value);
    void from_json(const nlohmann::json& json, GLRectangleShape& value);

    class GLTextShape : public GLPathShape
    {
    public:
        GLTextShape(const std::shared_ptr<image::FontSystem> f) :
            GLPathShape(),
            fontSize(30),
            fontSystem(f){};
        virtual ~GLTextShape(){};

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

    public:
        std::string fontFamily = "NotoSans-Regular";
        std::string txt;
        std::string text;
        uint16_t fontSize;
        std::shared_ptr<image::FontSystem> fontSystem;
    };

    void to_json(nlohmann::json& json, const GLTextShape& value);
    void from_json(const nlohmann::json& json, GLTextShape& value);

#ifdef USE_OPENGL2
    class GL2TextShape : public GLPathShape
    {
    public:
        GL2TextShape() :
            GLPathShape(){};
        virtual ~GL2TextShape(){};

        //! Auxiliary function to set the raster coordinates with no clipping
        bool setRasterPos(double x, double y, size_t textLength);

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;

    public:
        std::string txt; // Copy of text as we are processing it.
        std::string text;
        Fl_Font font;
        uint16_t fontSize = 30;
        int w;
        int h;
        double pixels_per_unit = 1.F;
        double viewZoom = 1.F;
    };

    void to_json(nlohmann::json& json, const GL2TextShape& value);
    void from_json(const nlohmann::json& json, GL2TextShape& value);

#endif

    class GLErasePathShape : public GLPathShape
    {
    public:
        GLErasePathShape() :
            GLPathShape(){};
        virtual ~GLErasePathShape(){};

        void draw(
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<opengl::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const GLErasePathShape& value);
    void from_json(const nlohmann::json& json, GLErasePathShape& value);

    typedef std::vector< std::shared_ptr< draw::Shape > > ShapeList;
} // namespace mrv
