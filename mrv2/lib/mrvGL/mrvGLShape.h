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

namespace mrv
{
    using namespace tl;

    class GLShape : public tl::draw::Shape
    {
    public:
        GLShape() :
            tl::draw::Shape()
        {
        }
        virtual ~GLShape(){};

        virtual void draw(const std::shared_ptr<timeline::IRender>&) = 0;

    public:
        bool soft = false;
    };

    class GLCircleShape : public GLShape
    {
    public:
        GLCircleShape() :
            GLShape(),
            radius(1.0){};
        virtual ~GLCircleShape(){};

        void draw(const std::shared_ptr<timeline::IRender>&) override;

        math::Vector2i center;
        double radius;
    };

    void to_json(nlohmann::json& json, const GLCircleShape& value);
    void from_json(const nlohmann::json& json, GLCircleShape& value);

    class GLPathShape : public tl::draw::PathShape
    {
    public:
        GLPathShape() :
            tl::draw::PathShape(){};
        virtual ~GLPathShape(){};

        void draw(const std::shared_ptr<timeline::IRender>&) override;

    public:
        bool soft = false;
    };

    void to_json(nlohmann::json& json, const GLPathShape& value);
    void from_json(const nlohmann::json& json, GLPathShape& value);

    class GLArrowShape : public GLPathShape
    {
    public:
        GLArrowShape() :
            GLPathShape(){};
        virtual ~GLArrowShape(){};

        void draw(const std::shared_ptr<timeline::IRender>&) override;
    };

    void to_json(nlohmann::json& json, const GLArrowShape& value);
    void from_json(const nlohmann::json& json, GLArrowShape& value);

    class GLRectangleShape : public GLPathShape
    {
    public:
        GLRectangleShape() :
            GLPathShape(){};
        void draw(const std::shared_ptr<timeline::IRender>&) override;
        virtual ~GLRectangleShape(){};
    };

    void to_json(nlohmann::json& json, const GLRectangleShape& value);
    void from_json(const nlohmann::json& json, GLRectangleShape& value);

    class GLTextShape : public GLPathShape
    {
    public:
        GLTextShape(const std::shared_ptr<imaging::FontSystem> f) :
            GLPathShape(),
            fontSize(30),
            fontSystem(f){};
        virtual ~GLTextShape(){};

        void draw(const std::shared_ptr<timeline::IRender>&) override;

    public:
        std::string fontFamily = "NotoSans-Regular";
        std::string txt;
        std::string text;
        uint16_t fontSize;
        std::shared_ptr<imaging::FontSystem> fontSystem;
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

        void draw(const std::shared_ptr<timeline::IRender>&) override;

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

        void draw(const std::shared_ptr<timeline::IRender>&) override;
    };

    void to_json(nlohmann::json& json, const GLErasePathShape& value);
    void from_json(const nlohmann::json& json, GLErasePathShape& value);

    typedef std::vector< std::shared_ptr< tl::draw::Shape > > ShapeList;
} // namespace mrv
