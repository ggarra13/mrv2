// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvVk/mrvVkDefines.h"
#include "mrvVk/mrvVkLines.h"

#ifdef TLRENDER_FFMPEG
#  include "mrvVoice/mrvVoiceOver.h"
#endif

#include <tlTimelineVk/Render.h>

#include <tlDraw/Shape.h>

#include <tlCore/Matrix.h>

#include <FL/Enumerations.H>

#include <cmath>
#include <filesystem>
#include <limits>
#include <vector>
#include <iostream>
namespace fs = std::filesystem;

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
        VKTextShape() :
            VKPathShape(),
            fontSize(30)
            {
            };
        virtual ~VKTextShape() {};

        int accept();
        
        int handle(int event);
        
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;
        
        int handle_mouse_click(int event, const math::Vector2i& pos);

    protected:
        int kf_select_all();
        int kf_copy();
        int kf_copy_cut();
        int kf_paste();
        void to_cursor();
        int handle_insert(const char* new_text, int len, int del);
        int handle_delete();
        int handle_backspace();
        int handle_move_up();
        int handle_move_down();
        int handle_move_right();
        int handle_move_left();
        unsigned line_start(unsigned utf8);
        unsigned line_end(unsigned utf8);
        unsigned current_line();
        unsigned current_column();
        const char* advance_to_column(unsigned start, unsigned column);
        
    public:
        std::string fontPath;
        //std::string text = "🎃";
        std::string text;
        uint16_t fontSize;
        bool editing = true;
        Fl_Font font;
        unsigned utf8_pos = 0;
        unsigned cursor = 0;
        math::Vector2f pos;
        math::Box2i    box;
        float mult     = 1.F;
        float viewZoom = 1.F;
        std::shared_ptr<image::FontSystem> fontSystem;
    };

    void to_json(nlohmann::json& json, const VKTextShape& value);
    void from_json(const nlohmann::json& json, VKTextShape& value);



    class VKLinkShape : public VKPathShape
    {
    public:
        VKLinkShape() :
            VKPathShape() {};
        virtual ~VKLinkShape() {};

        void open();
        bool edit();
        int handle(int event);

        const math::Box2f getBBox(float mult) const;
        
        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;

    public:
        std::string url;
        std::string title;
        float mult = 1;
    };

    void to_json(nlohmann::json& json, const VKLinkShape& value);
    void from_json(const nlohmann::json& json, VKLinkShape& value);
    
    class VKErasePathShape : public VKPathShape
    {
    public:
        VKErasePathShape() :
            VKPathShape() {};
        virtual ~VKErasePathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_vlk::Render>&,
            const std::shared_ptr<vulkan::Lines> lines) override;

        bool rectangle = false;
        bool drawing = false;
        float mult = 1.F;
    };

    void to_json(nlohmann::json& json, const VKErasePathShape& value);
    void from_json(const nlohmann::json& json, VKErasePathShape& value);

    typedef std::vector< std::shared_ptr< draw::Shape > > ShapeList;

#ifdef TLRENDER_FFMPEG
    /** 
     * Auxiliary class used to draw a voice over icon.
     * 
     */
    class VKVoiceOverShape
    {
    public:
        VKVoiceOverShape()
            {
            };
        ~VKVoiceOverShape() {};
        
        void draw(const std::shared_ptr<timeline_vlk::Render>&,
                  const voice::MouseData& mouse);
        
        voice::RecordStatus status;
        math::Vector2f center;
        float mult = 1;
        unsigned blinkingIndex = 0;
    };
#endif

} // namespace mrv
