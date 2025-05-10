// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvVk/mrvVkDefines.h"
#include "mrvVk/mrvVkLines.h"

#include <tlTimelineVk/Render.h>

#include <tlDraw/Shape.h>

#include <tlCore/Matrix.h>

#include <FL/Enumerations.H>

#include <limits>
#include <cmath>
#include <vector>
#include <iostream>

namespace mrv
{
    using namespace tl;

    class VKShape : public draw::Shape
    {
    public:
        VKShape() :
            draw::Shape()
        {
        }
        virtual ~VKShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) = 0;
    };

    class VKCircleShape : public VKShape
    {
    public:
        VKCircleShape() :
            VKShape(),
            radius(1.0) {};
        virtual ~VKCircleShape() {};
        
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;

        math::Vector2f center;
        double radius;
    };

    void to_json(nlohmann::json& json, const VKCircleShape& value);
    void from_json(const nlohmann::json& json, VKCircleShape& value);

    class VKFilledCircleShape : public VKCircleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKFilledCircleShape& value);
    void from_json(const nlohmann::json& json, VKFilledCircleShape& value);

    class VKPathShape : public draw::PathShape
    {
    public:
        VKPathShape() :
            draw::PathShape() {};
        virtual ~VKPathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines);
    };

    void to_json(nlohmann::json& json, const VKPathShape& value);
    void from_json(const nlohmann::json& json, VKPathShape& value);

    class VKPolygonShape : public VKPathShape
    {
    public:
        VKPolygonShape() :
            VKPathShape() {};
        virtual ~VKPolygonShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    class VKFilledPolygonShape : public VKPolygonShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKFilledPolygonShape& value);
    void from_json(const nlohmann::json& json, VKFilledPolygonShape& value);

    class VKArrowShape : public VKPathShape
    {
    public:
        VKArrowShape() :
            VKPathShape() {};
        virtual ~VKArrowShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKArrowShape& value);
    void from_json(const nlohmann::json& json, VKArrowShape& value);

    class VKRectangleShape : public VKPathShape
    {
    public:
        VKRectangleShape() :
            VKPathShape() {};
        virtual ~VKRectangleShape() {};
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKRectangleShape& value);
    void from_json(const nlohmann::json& json, VKRectangleShape& value);

    class VKFilledRectangleShape : public VKRectangleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKFilledRectangleShape& value);
    void from_json(const nlohmann::json& json, VKFilledRectangleShape& value);

    class VKTextShape : public VKPathShape
    {
    public:
        VKTextShape(const std::shared_ptr<image::FontSystem> f) :
            VKPathShape(),
            fontSize(30),
            fontSystem(f) {};
        virtual ~VKTextShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;

    public:
        std::string fontFamily = "NotoSans-Regular";
        std::string txt;
        std::string text;
        uint16_t fontSize;
        Fl_Font font;
        float viewZoom = 1.F;
        std::shared_ptr<image::FontSystem> fontSystem;
    };

    void to_json(nlohmann::json& json, const VKTextShape& value);
    void from_json(const nlohmann::json& json, VKTextShape& value);

#ifdef USE_OPENVK2
    class VK2TextShape : public VKPathShape
    {
    public:
        VK2TextShape() :
            VKPathShape() {};
        virtual ~VK2TextShape() {};

        //! Auxiliary function to set the raster coordinates with no clipping
        bool setRasterPos(double x, double y, size_t textLength);

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;

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

    void to_json(nlohmann::json& json, const VK2TextShape& value);
    void from_json(const nlohmann::json& json, VK2TextShape& value);

#endif

    class VKErasePathShape : public VKPathShape
    {
    public:
        VKErasePathShape() :
            VKPathShape() {};
        virtual ~VKErasePathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
    };

    void to_json(nlohmann::json& json, const VKErasePathShape& value);
    void from_json(const nlohmann::json& json, VKErasePathShape& value);

    typedef std::vector< std::shared_ptr< draw::Shape > > ShapeList;
} // namespace mrv
