// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvWGPU/mrvWGPUDefines.h"
#include "mrvWGPU/mrvWGPULines.h"

#include "mrvVoice/mrvVoiceOver.h"

#include <tlCore/Matrix.h>

#include <tlTimelineWGPU/Render.h>

#include <tlDraw/Shape.h>

#include <FL/Enumerations.H>

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>

namespace mrv
{
    using namespace tl;

    class WGPUShape : public draw::Shape
    {
    public:
        WGPUShape() :
            draw::Shape()
        {
        }
        virtual ~WGPUShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) = 0;
    };

    class WGPUCircleShape : public WGPUShape
    {
    public:
        WGPUCircleShape() :
            WGPUShape(),
            radius(1.0) {};
        virtual ~WGPUCircleShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;

        math::Vector2f center;
        double radius;
    };

    void to_json(nlohmann::json& json, const WGPUCircleShape& value);
    void from_json(const nlohmann::json& json, WGPUCircleShape& value);

    class WGPUFilledCircleShape : public WGPUCircleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPUFilledCircleShape& value);
    void from_json(const nlohmann::json& json, WGPUFilledCircleShape& value);

    class WGPUPathShape : public draw::PathShape
    {
    public:
        WGPUPathShape() :
            draw::PathShape() {};
        virtual ~WGPUPathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&);
    };

    void to_json(nlohmann::json& json, const WGPUPathShape& value);
    void from_json(const nlohmann::json& json, WGPUPathShape& value);

    class WGPUPolygonShape : public WGPUPathShape
    {
    public:
        WGPUPolygonShape() :
            WGPUPathShape() {};
        virtual ~WGPUPolygonShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPUPolygonShape& value);
    void from_json(const nlohmann::json& json, WGPUPolygonShape& value);

    class WGPUFilledPolygonShape : public WGPUPolygonShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPUFilledPolygonShape& value);
    void from_json(const nlohmann::json& json, WGPUFilledPolygonShape& value);

    class WGPUArrowShape : public WGPUPathShape
    {
    public:
        WGPUArrowShape() :
            WGPUPathShape() {};
        virtual ~WGPUArrowShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPUArrowShape& value);
    void from_json(const nlohmann::json& json, WGPUArrowShape& value);

    class WGPURectangleShape : public WGPUPathShape
    {
    public:
        WGPURectangleShape() :
            WGPUPathShape() {};
        virtual ~WGPURectangleShape() {};
        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPURectangleShape& value);
    void from_json(const nlohmann::json& json, WGPURectangleShape& value);

    class WGPUFilledRectangleShape : public WGPURectangleShape
    {
    public:
        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;
    };

    void to_json(nlohmann::json& json, const WGPUFilledRectangleShape& value);
    void from_json(const nlohmann::json& json, WGPUFilledRectangleShape& value);

    class WGPUTextShape : public WGPUPathShape
    {
    public:
        WGPUTextShape() :
            WGPUPathShape(),
            fontSize(30)
            {
            };
        virtual ~WGPUTextShape() {};

        int accept();

        int handle(int event);

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>& lines) override;

        int handle_mouse_click(int event, const math::Vector2i& pos);

    protected:
        void _positionCursor(
            const std::string& line, const char* text_it, int x, int y,
            const math::Vector2i& local,
            math::Vector2i& cursor_pos);
        void _drawLine(
            const std::shared_ptr<timeline_wgpu::Render>& render,
            const std::string& line, int x, int y,
            std::vector<timeline::TextInfo>& textInfos,
            unsigned& cursor_count,
            math::Vector2i& cursor_pos);
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

    void to_json(nlohmann::json& json, const WGPUTextShape& value);
    void from_json(const nlohmann::json& json, WGPUTextShape& value);


    class WGPULinkShape : public WGPUPathShape
    {
   public:
        WGPULinkShape() :
            WGPUPathShape() {};
        virtual ~WGPULinkShape() {};

        const math::Box2f getBBox(float scale) const;

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;

        void open();
        bool edit();
        int handle(int event);

    public:
        std::string url;
        std::string title;
        std::string ai_prompt;
        float mult = 1.F;
    };

    void to_json(nlohmann::json& json, const WGPULinkShape& value);
    void from_json(const nlohmann::json& json, WGPULinkShape& value);


    class WGPUErasePathShape : public WGPUPathShape
    {
    public:
        WGPUErasePathShape() :
            WGPUPathShape() {};
        virtual ~WGPUErasePathShape() {};

        virtual void draw(
            const std::shared_ptr<timeline_wgpu::Render>&,
            const std::shared_ptr<webgpu::Lines>&) override;

        bool rectangle = false;
        bool drawing = true;
        float mult = 1.F;
    };

    void to_json(nlohmann::json& json, const WGPUErasePathShape& value);
    void from_json(const nlohmann::json& json, WGPUErasePathShape& value);

    typedef std::vector< std::shared_ptr< draw::Shape > > ShapeList;


#ifndef __EMSCRIPTEN__
    /**
     * Auxiliary class used to draw a voice over icon.
     *
     */
    class WGPUVoiceOverShape
    {
    public:
        WGPUVoiceOverShape()
            {
            };
        ~WGPUVoiceOverShape() {};

        void draw(const std::shared_ptr<timeline::IRender>&,
                  const voice::MouseData& mouse);

        voice::RecordStatus status;
        math::Vector2f center;
        float mult = 1.F;
    };
#endif

} // namespace mrv
