// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <memory>
#include <cmath>
#include <algorithm>

#include <FL/names.h>
#include <FL/filename.H>
#include <FL/Fl_Menu_Button.H>

#include "mrViewer.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvGL/mrvGLShape.h"

#include "mrvPanels/mrvComparePanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvLaserFadeData.h"

// #define DEBUG_EVENTS

namespace
{
    const char* kModule = "view";
    const int kCrossSize = 10;
} // namespace

namespace mrv
{

    namespace
    {
        inline int signbit(int num)
        {
            int sign = 0;
            if (num > 0)
                sign = 1;
            else if (num < 0)
                sign = -1;
            return sign;
        }

        bool isDrawAction(ActionMode mode)
        {
            switch(mode)
            {
            case ActionMode::kArrow:
            case ActionMode::kCircle:
            case ActionMode::kDraw:
            case ActionMode::kErase:
            case ActionMode::kFilledCircle:
            case ActionMode::kFilledRectangle:
            case ActionMode::kFilledPolygon:
            case ActionMode::kPolygon:
            case ActionMode::kRectangle:
                return true;
            default:
                return false;
            }
        }
        
    } // namespace


    namespace opengl
    {

        void TimelineViewport::_handleDragLeftMouseButton() noexcept
        {
            TLRENDER_P();

            if (p.compareOptions.mode == timeline::CompareMode::Wipe)
            {
                _handleCompareWipe();
            }
            else if (p.compareOptions.mode == timeline::CompareMode::Overlay)
            {
                _handleCompareOverlay();
            }
            else
            {
                if (p.actionMode == ActionMode::kScrub ||
                    (p.actionMode == ActionMode::kRotate && _isEnvironmentMap()))
                {
                    p.lastEvent = FL_DRAG;

                    const auto& pos = _getFocus();
                    int dx = pos.x - p.mousePress.x;

                    if (Fl::event_shift() && _isEnvironmentMap())
                    {
                        const float speed = _getZoomSpeedValue();
                        auto o = p.environmentMapOptions;
                        o.focalLength += dx * speed;
                        p.mousePress = pos;
                        setEnvironmentMapOptions(o);
                    }
                    else
                    {
                        if (Fl::event_shift())
                        {
                            return _handleDragSelection();
                        }
                        else
                        {
                            p.isScrubbing = true;
                            if (Fl::event_alt())
                            {
                                float multiplier = p.ui->uiPrefs->uiPrefsAltScrubbingSensitivity->value();
                                scrub(multiplier);
                            }
                            else
                            {
                                scrub();
                            }
                        }
                    }
                    return;
                }
                else if (
                    Fl::event_shift() || p.actionMode == ActionMode::kSelection)
                {
                    _handleDragSelection();
                    return;
                }
                else
                {
                    draw::Point pnt(_getRasterf());

                    auto player = getTimelinePlayer();
                    if (!player)
                        return;

                    auto annotation = player->getAnnotation();
                    if (p.actionMode != ActionMode::kScrub && !annotation)
                        return;

                    if (isDrawAction(p.actionMode) &&
                        !p.showAnnotations)
                    {
                        p.showAnnotations = true;
                    }
                    
                    std::shared_ptr< draw::Shape > s;
                    if (annotation)
                        s = annotation->lastShape();

                    switch (p.actionMode)
                    {
                    case ActionMode::kScrub:
                        if (Fl::event_alt())
                        {
                            if (!p.player)
                                return;

                            const int X = Fl::event_x() * pixels_per_unit();
                            const float scale = p.ui->uiPrefs->uiPrefsScrubbingSensitivity->value() * 20;
                            float dx = (X - p.mousePress.x) / scale;
                            
                            if (std::abs(dx) >= 1.0F)
                            {
                                p.isScrubbing = true;
                                _scrub(dx);
                                p.mousePress.x = X;
                            }
                        }
                        else
                        {
                            scrub();
                        }
                        return;
                    case ActionMode::kRectangle:
                    case ActionMode::kFilledRectangle:
                    {
                        auto shape = dynamic_cast< GLRectangleShape* >(s.get());
                        if (!shape)
                            return;

                        shape->pts[1].x = pnt.x;
                        shape->pts[2].x = pnt.x;
                        shape->pts[2].y = pnt.y;
                        shape->pts[3].y = pnt.y;
                        _updateAnnotationShape();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kPolygon:
                    case ActionMode::kFilledPolygon:
                    {
                        auto shape = dynamic_cast< GLPathShape* >(s.get());
                        if (!shape)
                            return;

                        auto& lastPoint = shape->pts.back();
                        lastPoint = pnt;

                        _updateAnnotationShape();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kDraw:
                    {
                        auto shape = dynamic_cast< GLPathShape* >(s.get());
                        if (!shape)
                            return;

                        shape->pts.push_back(pnt);
                        _addAnnotationShapePoint();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kErase:
                    {
                        auto shape = dynamic_cast< GLErasePathShape* >(s.get());
                        if (!shape)
                            return;

                        shape->pts.push_back(pnt);
                        _addAnnotationShapePoint();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kArrow:
                    {
                        auto shape = dynamic_cast< GLArrowShape* >(s.get());
                        if (!shape)
                            return;

                        Imath::V2d p1 = shape->pts[0];
                        Imath::V2d lineVector = pnt - p1;
                        double lineLength = lineVector.length();

                        const float theta = 45 * M_PI / 180;
                        const double nWidth = lineLength * 0.2;

                        const double tPointOnLine =
                            nWidth / (2 * (tan(theta) / 2) * lineLength);
                        const Imath::V2d& pointOnLine =
                            pnt + -tPointOnLine * lineVector;

                        const Imath::V2d normalVector(-lineVector.y, lineVector.x);

                        const double tNormal = nWidth / (2 * lineLength);
                        Imath::V2d tmp = pointOnLine + tNormal * normalVector;

                        shape->pts[1] = pnt;
                        shape->pts[2] = tmp;
                        shape->pts[3] = pnt;
                        tmp = pointOnLine + -tNormal * normalVector;
                        shape->pts[4] = tmp;
                        _updateAnnotationShape();

                        redrawWindows();
                        return;
                    }
                    case ActionMode::kFilledCircle:
                    case ActionMode::kCircle:
                    {
                        auto shape = dynamic_cast< GLCircleShape* >(s.get());
                        if (!shape)
                            return;

                        shape->radius =
                            2.0F * abs(shape->center.x - pnt.x) * pixels_per_unit();
                        if (shape->radius < shape->pen_size / 2)
                            shape->radius = shape->pen_size / 2;
                        _updateAnnotationShape();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kText:
                    {
                        MultilineInput* w = getMultilineInput();
                        if (w)
                        {
                            auto pos = math::Vector2i(p.event_x, p.event_y);
#ifdef USE_OPENGL2
                            w->pos = pos;
#else
                            w->Fl_Widget::position(pos.x, pos.y);
#endif
                            redrawWindows();
                        }
                        return;
                    }
                    default:
                        LOG_ERROR(_("Unknown action mode in ") << __FUNCTION__);
                        return;
                    }
                }
            }
        }

        MultilineInput* TimelineViewport::getMultilineInput() const noexcept
        {
            MultilineInput* w;
            for (int i = children(); i--;)
            {
                w = dynamic_cast< MultilineInput* >(child(i));
                if (!w)
                    continue;
                return w;
            }
            return nullptr;
        }

        int TimelineViewport::acceptMultilineInput() noexcept
        {
            TLRENDER_P();

            MultilineInput* w = getMultilineInput();
            if (!w)
                return 0;

            int ret = 0;
            const char* text = w->value();
            if (text && strlen(text) > 0)
            {
                auto player = getTimelinePlayer();
                if (!player)
                    return 0;

                auto annotation = player->getAnnotation();
                if (!annotation)
                    return 0;

                uint8_t r, g, b;
                int fltk_color = p.ui->uiPenColor->color();
                float alpha = p.ui->uiPenOpacity->value();
                Fl::get_color((Fl_Color)fltk_color, r, g, b);
                const image::Color4f color(r / 255.F, g / 255.F, b / 255.F, alpha);
                fl_font(w->textfont(), w->textsize());

#ifdef USE_OPENGL2
                auto shape = std::make_shared< GL2TextShape >();
#else
                auto shape = std::make_shared< GLTextShape >(p.fontSystem);
#endif

                draw::Point pnt(_getRasterf());
                shape->pts.push_back(pnt); // we'll change it later...

                annotation->push_back(shape);

                // Calculate offset from corner due to cross and the bottom of
                // the font.
                const math::Vector2i offset(
                    kCrossSize + 2, kCrossSize + fl_height() - fl_descent());

                shape->text = text;
                shape->color = color;

                const float pixels_unit = pixels_per_unit();

#ifdef USE_OPENGL2
                shape->font = w->textfont();
                shape->fontSize = w->textsize() / p.viewZoom * pixels_unit;

                auto pos = math::Vector2i(w->x() + offset.x, w->y() + offset.y);
                pnt = _getFocusf(pos.x, pos.y);

                // Store rotation
                float rotation = p.rotation;
                float videoRotation = p.videoRotation;

                p.rotation = 0.F;
                p.videoRotation = 0.F;
                pnt = _getRasterf(pnt.x, pnt.y);

                // Restore rotation
                p.rotation = rotation;
                p.videoRotation = videoRotation;

                // Save new shape position
                shape->pts[0].x = pnt.x;
                shape->pts[0].y = pnt.y;
#else
                shape->fontFamily = w->fontFamily;
                shape->fontSize = w->textsize() / p.viewZoom * pixels_unit;

                // @bug: this is broken on image rotations
                shape->pts[0].x += offset.x;
                shape->pts[0].y -= offset.y;
                shape->pts[0].y = -shape->pts[0].y;
#endif
                _endAnnotationShape();
                p.ui->uiUndoDraw->activate();
                ret = 1;
            }

            // Safely delete the winget.  This call removes the
            // widget from the opengl canvas too.
            Fl::delete_widget(w);
            redrawWindows();
            take_focus();
            return ret;
        }

        void TimelineViewport::_handlePushLeftMouseButton() noexcept
        {
            TLRENDER_P();

            p.playbackMode = timeline::Playback::Stop;

            if (p.player)
                p.playbackMode = p.player->playback();

            if (p.compareOptions.mode == timeline::CompareMode::Wipe)
            {
                _handleCompareWipe();
            }
            else if (p.compareOptions.mode == timeline::CompareMode::Overlay)
            {
                _handleCompareOverlay();
            }
            else
            {
                if (Fl::event_shift() || p.actionMode == ActionMode::kSelection)
                {
                    p.lastEvent = FL_DRAG;
                    p.mousePos = _getFocus();
                    math::Vector2i pos = _getRaster();

                    _clipSelectionArea(pos);
                    math::Box2i area;
                    area.min = pos;
                    area.max = pos;
                    setSelectionArea(area);
                    redrawWindows();
                }
                else
                {

                    if (p.actionMode == ActionMode::kScrub ||
                        p.actionMode == ActionMode::kRotate)
                    {
                        if (!p.isScrubbing && p.player &&
                            p.actionMode == ActionMode::kScrub)
                        {
                            p.isScrubbing = true;
                            p.player->setPlayback(timeline::Playback::Stop,
                                                  p.isScrubbing);
                        }
                        
                        p.lastEvent = FL_PUSH;
                        return;
                    }

                    uint8_t r, g, b;
                    SettingsObject* settings = p.ui->app->settings();
                    int fltk_color = p.ui->uiPenColor->color();
                    Fl::get_color((Fl_Color)fltk_color, r, g, b);
                    float alpha = p.ui->uiPenOpacity->value();
                    const image::Color4f color(
                        r / 255.F, g / 255.F, b / 255.F, alpha);
                    const float pen_size = _getPenSize();

                    bool laser = settings->getValue<bool>(kLaser);
                    bool softBrush = settings->getValue<bool>(kSoftBrush);
                    Fl_Font font =
                        static_cast<Fl_Font>(settings->getValue<int>(kTextFont));

                    p.mousePos = _getFocus();
                    draw::Point pnt(_getRasterf());

                    auto player = getTimelinePlayer();
                    if (!player)
                        return;

                    auto annotation = player->getAnnotation();
                    bool all_frames =
                        p.ui->app->settings()->getValue<bool>(kAllFrames);
                    if (!annotation)
                    {
                        annotation = player->createAnnotation(all_frames);
                        if (!annotation)
                            return;
                    }
                    else
                    {
                        if (annotation->allFrames != all_frames)
                        {
                            std::string error;
                            if (all_frames)
                            {
                                error +=
                                    _("Cannot create an annotation here for all "
                                      "frames.  "
                                      "A current frame annotation already "
                                      "exists.");
                            }
                            else
                            {
                                error +=
                                    _("Cannot create an annotation here for "
                                      "current frame.  "
                                      "An all frames annotation already exists.");
                            }
                            LOG_ERROR(error);
                            return;
                        }
                    }

                    switch (p.actionMode)
                    {
                    case ActionMode::kDraw:
                    {
                        auto shape = std::make_shared< GLPathShape >();
                        shape->pen_size = pen_size;
                        shape->color = color;
                        shape->soft = softBrush;
                        shape->laser = laser;
                        shape->pts.push_back(pnt);
                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kErase:
                    {
                        auto shape = std::make_shared< GLErasePathShape >();
                        shape->pen_size = pen_size * 3.5F;
                        shape->color = color;
                        shape->soft = softBrush;
                        shape->pts.push_back(pnt);
                        annotation->push_back(shape);
                        _createAnnotationShape(false);
                        break;
                    }
                    case ActionMode::kArrow:
                    {
                        auto shape = std::make_shared< GLArrowShape >();
                        shape->pen_size = pen_size;
                        shape->soft = softBrush;
                        shape->color = color;
                        shape->laser = laser;
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kFilledPolygon:
                    {
                        std::shared_ptr<GLFilledPolygonShape> shape;
                        if (p.lastEvent != FL_DRAG)
                        {
                            shape = std::make_shared< GLFilledPolygonShape >();
                            shape->pen_size = pen_size;
                            shape->soft = false;
                            shape->color = color;
                            shape->laser = laser;
                            shape->pts.push_back(pnt);
                            p.lastEvent = FL_PUSH;
                        }
                        else
                        {
                            auto s = annotation->lastShape();
                            shape =
                                std::dynamic_pointer_cast<GLFilledPolygonShape>(s);
                            if (!shape)
                                return;
                            shape->pts.push_back(pnt);
                        }
                    
                        if (p.lastEvent == FL_PUSH)
                        {
                            annotation->push_back(shape);
                            _createAnnotationShape(false);
                            p.lastEvent = FL_DRAG;
                        }
                        break;
                    }
                    case ActionMode::kPolygon:
                    {
                        std::shared_ptr<GLPolygonShape> shape;
                        if (p.lastEvent != FL_DRAG)
                        {
                            shape = std::make_shared< GLPolygonShape >();
                            shape->pen_size = pen_size;
                            shape->soft = false;
                            shape->color = color;
                            shape->laser = laser;
                            p.lastEvent = FL_PUSH;
                        }
                        else
                        {
                            auto s = annotation->lastShape();
                            shape = std::dynamic_pointer_cast<GLPolygonShape>(s);
                            if (!shape)
                                return;
                        }
                    
                        shape->pts.push_back(pnt);
                        
                        if (p.lastEvent == FL_PUSH)
                        {
                            annotation->push_back(shape);
                            _createAnnotationShape(false);
                            p.lastEvent = FL_DRAG;
                        }
                        break;
                    }
                    case ActionMode::kFilledCircle:
                    {
                        auto shape = std::make_shared< GLFilledCircleShape >();
                        shape->pen_size = pen_size;
                        shape->soft = softBrush;
                        shape->color = color;
                        shape->laser = laser;
                        shape->center = _getRasterf();
                        shape->radius = 0;

                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kCircle:
                    {
                        auto shape = std::make_shared< GLCircleShape >();
                        shape->pen_size = pen_size;
                        shape->soft = softBrush;
                        shape->color = color;
                        shape->laser = laser;
                        shape->center = _getRasterf();
                        shape->radius = 0;

                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kFilledRectangle:
                    {
                        auto shape = std::make_shared< GLFilledRectangleShape >();
                        shape->pen_size = pen_size;
                        shape->soft = softBrush;
                        shape->color = color;
                        shape->laser = laser;
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kRectangle:
                    {
                        auto shape = std::make_shared< GLRectangleShape >();
                        shape->pen_size = pen_size;
                        shape->soft = softBrush;
                        shape->color = color;
                        shape->laser = laser;
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        shape->pts.push_back(pnt);
                        annotation->push_back(shape);
                        _createAnnotationShape(laser);
                        break;
                    }
                    case ActionMode::kText:
                    {
                        const auto& renderSize = getRenderSize();
                        float pct = renderSize.h / 1024.F;
                        auto w = getMultilineInput();

                        int font_size = settings->getValue<int>(kFontSize);
                        double fontSize =
                            font_size * pct * p.viewZoom / pixels_per_unit();
                        math::Vector2i pos(p.event_x, p.event_y);
                        if (w)
                        {
                            w->take_focus();
                            w->pos = pos;
#ifdef USE_OPENGL2
                            w->textfont((Fl_Font)font);
#else
                            w->Fl_Widget::position(pos.x, pos.y);
#endif
                            w->textsize(fontSize);
                            redrawWindows();
                            return;
                        }

                        w = new MultilineInput(
                            pos.x, pos.y, 20, 30 * pct * p.viewZoom);
                        w->take_focus();
#ifdef USE_OPENGL2
                        w->textfont((Fl_Font)font);
#endif
                        w->textsize(fontSize);
                        w->textcolor(fltk_color);
                        w->viewPos = p.viewPos;
                        w->viewZoom = p.viewZoom;
                        w->redraw();

                        this->add(w);

                        redrawWindows();
                        return;
                    }
                    default:
                        return;
                    }
                    // Create annotation menus if not there already
                    App::unsaved_annotations = true;
                    p.ui->uiMain->update_title_bar();
                    p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
                    p.ui->uiUndoDraw->activate();
                }
            }
        }


        void TimelineViewport::editText(
            const std::shared_ptr< draw::Shape >& s, const int index) noexcept
        {
            TLRENDER_P();

            auto player = getTimelinePlayer();
            if (!player)
                return;

            auto annotation = player->getAnnotation();
            if (!annotation)
                return;

            const auto& renderSize = getRenderSize();
            float pct = renderSize.h / 1024.F;

            const float devicePixelRatio = pixels_per_unit();

#ifdef USE_OPENGL2
            auto shape = dynamic_cast<GL2TextShape*>(s.get());
            if (!shape)
                return;

            const int fontsize =
                shape->fontSize * shape->viewZoom / devicePixelRatio;
            fl_font(shape->font, fontsize);

            const math::Vector2i offset(
                kCrossSize + 2, kCrossSize + fl_height() - fl_descent());

            math::Vector2f pos(
                shape->pts[0].x * shape->viewZoom + p.viewPos.x,
                shape->pts[0].y * shape->viewZoom + p.viewPos.y);
            pos.x = pos.x / devicePixelRatio;
            pos.y = h() - 1 - pos.y / devicePixelRatio;

            pos.x -= offset.x;
            pos.y -= offset.y;

            auto w = new MultilineInput(pos.x, pos.y, 20, 30 * pct * p.viewZoom);
            w->take_focus();
            w->textfont(shape->font);
            w->textsize(shape->fontSize);
            w->value(shape->text.c_str(), shape->text.size());
            w->recalc();
#else
            auto shape = dynamic_cast<GLTextShape*>(s.get());
            if (!shape)
                return;

            const int fontsize =
                shape->fontSize * shape->viewZoom / devicePixelRatio;
            fl_font(shape->font, fontsize);

            const math::Vector2i offset(
                kCrossSize + 2, kCrossSize + fl_height() - fl_descent());

            math::Vector2i pos(
                shape->pts[0].x + p.viewPos.x, -shape->pts[0].y - p.viewPos.y);

            pos.x -= offset.x;
            pos.y -= offset.y;

            auto w = new MultilineInput(pos.x, pos.y, 20, 30 * pct * p.viewZoom);
            w->take_focus();
            w->fontFamily = shape->fontFamily;
            w->textsize(shape->fontSize);
            w->value(shape->text.c_str(), shape->text.size());
            w->recalc();
#endif
            Fl_Color fltk_color = fl_rgb_color(
                shape->color.r * 255.F, shape->color.g * 255.F,
                shape->color.b * 255.F);
            w->textcolor(fltk_color);
            w->viewPos = p.viewPos;
            w->viewZoom = p.viewZoom;
            w->redraw();

            tcp->pushMessage("Remove Shape", index);

            annotation->remove(s);
            this->add(w);

            p.ui->uiTimeline->redraw();
            redrawWindows();
        }

    } // namespace opengl

} // namespace mrv
