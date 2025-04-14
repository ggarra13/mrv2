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

#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"

#include "mrvPanels/mrvComparePanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvIO.h"

// #define DEBUG_EVENTS

namespace
{
    const char* kModule = "view";
    const int kCrossSize = 10;

    const float kSpinTimeout = 0.025;
    const float kSpinSumX = 0.05;
    const float kSpinSumY = 0.05;
    const float kSpinMaxY = 2.0;
    const float kSpinMaxX = 1.0;

    const float kLaserFadeTimeout = 0.01;
    const float kLaserFade = 0.025;

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
    } // namespace

    void TimelineViewport::laserFade_cb(LaserFadeData* data)
    {
        TimelineViewport* view = data->view;
        view->laserFade(data);
    }

    void TimelineViewport::laserFade(LaserFadeData* data)
    {
        auto s = data->shape;
        s->fade -= kLaserFade;
        if (s->fade <= 0.F)
        {
            Fl::remove_timeout((Fl_Timeout_Handler)laserFade_cb, data);
            // Remove shape from list
            data->annotation->remove(s);
            if (data->annotation->empty())
            {
                auto player = getTimelinePlayer();
                if (!player)
                    return;
                player->removeAnnotation(data->annotation);
                updateUndoRedoButtons();
            }
            // Remove callback data
            delete data;
        }
        else
        {
            Fl::repeat_timeout(
                kLaserFadeTimeout, (Fl_Timeout_Handler)laserFade_cb, data);
        }
        redrawWindows();
    }

    void TimelineViewport::redrawWindows() const
    {
        _p->ui->uiView->redraw();
        if (_p->ui->uiSecondary && _p->ui->uiSecondary->window()->visible())
        {
            Viewport* view = _p->ui->uiSecondary->viewport();
            view->redraw();
        }
    }

    void TimelineViewport::_handleCompareOverlay() noexcept
    {
        TLRENDER_P();

        if (Fl::event_alt())
        {
            float dx = p.event_x / (float)w();
            p.compareOptions.overlay = dx;
            p.ui->app->filesModel()->setCompareOptions(p.compareOptions);
        }
    }

    void TimelineViewport::_handleCompareWipe() noexcept
    {
        TLRENDER_P();

        if (Fl::event_alt())
        {
            math::Vector2f pos = _getRasterf();
            const auto& renderSize = getRenderSize();
            
            float dx = pos.x / renderSize.w;
            float dy = pos.y / renderSize.h;

            p.compareOptions.wipeCenter.x = dx;
            p.compareOptions.wipeCenter.y = 1.0F - dy;
            p.ui->app->filesModel()->setCompareOptions(p.compareOptions);
            redrawWindows();
        }
        else if (Fl::event_shift())
        {
            float dx = p.event_x / (float)w() * 360.F;
            p.compareOptions.wipeRotation = dx;
            p.ui->app->filesModel()->setCompareOptions(p.compareOptions);
            redrawWindows();
        }
    }

    void TimelineViewport::_handleDragSelection() noexcept
    {
        TLRENDER_P();
        p.lastEvent = FL_DRAG;
        p.mousePos = _getFocus();
        math::Vector2i pos = _getRaster();

        _clipSelectionArea(pos);

        math::Box2i area = p.selection;
        area.max = pos;
        setSelectionArea(area);

        redrawWindows();
    }

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
                        return _handleDragSelection();
                    else
                        scrub();
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
                if (p.actionMode != kScrub && !annotation)
                    return;

                std::shared_ptr< draw::Shape > s;
                if (annotation)
                    s = annotation->lastShape();

                switch (p.actionMode)
                {
                case ActionMode::kScrub:
                    scrub();
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
                p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
                p.ui->uiUndoDraw->activate();
            }
        }
    }

    void TimelineViewport::_handleViewSpinning_cb(TimelineViewport* t) noexcept
    {
        t->handleViewSpinning();
    }

    void TimelineViewport::handleViewSpinning() noexcept
    {
        TLRENDER_P();
        bool changed = false;
        if (p.viewSpin.x >= kSpinSumX)
        {
            p.viewSpin.x -= kSpinSumX;
            changed = true;
        }
        else if (p.viewSpin.x <= -kSpinSumX)
        {
            p.viewSpin.x += kSpinSumX;
            changed = true;
        }
        else
            p.viewSpin.x = 0.0;

        if (p.viewSpin.y >= kSpinSumY)
        {
            p.viewSpin.y -= kSpinSumY;
            changed = true;
        }
        else if (p.viewSpin.y <= -kSpinSumY)
        {
            p.viewSpin.y += kSpinSumY;
            changed = true;
        }
        else
            p.viewSpin.y = 0.0;

        if (changed)
        {
            _updateViewRotation(p.viewSpin);
            Fl::repeat_timeout(
                kSpinTimeout, (Fl_Timeout_Handler)_handleViewSpinning_cb, this);
        }
    }

    void TimelineViewport::_handleDragMiddleMouseButton() noexcept
    {
        TLRENDER_P();

        set_cursor(FL_CURSOR_MOVE);
        if (p.actionMode == ActionMode::kRotate ||
            (p.actionMode == ActionMode::kScrub && _isEnvironmentMap()))
        {
            p.lastEvent = FL_DRAG;

            const auto& pos = _getFocus();
            int dx = pos.x - p.mousePress.x;

            bool changed = false;
            if (signbit(dx) != signbit(p.rotDir.x) && p.rotDir.x != 0)
            {
                p.rotDir.x = 0;
                p.mousePress.x = pos.x;
                changed = true;
            }

            const float speed = _getZoomSpeedValue();

            if (Fl::event_shift())
            {
                if (changed)
                    return;

                auto o = p.environmentMapOptions;
                o.focalLength += dx * speed;
                p.mousePress = pos;
                setEnvironmentMapOptions(o);
            }
            else
            {
                int dy = pos.y - p.mousePress.y;

                if (signbit(dy) != signbit(p.rotDir.y) && p.rotDir.y != 0)
                {
                    p.rotDir.y = 0;
                    p.mousePress.y = pos.y;
                    changed = true;
                }

                if (changed)
                    return;

                const auto viewportSize = getViewportSize();

                if (p.environmentMapOptions.spin)
                {
                    // x takes dy changes
                    p.viewSpin.x += double(-dy * speed) / viewportSize.h * 2;

                    // while y takes dx changes
                    p.viewSpin.y += double(dx * speed) / viewportSize.w * 2;

                    if (p.viewSpin.y > kSpinMaxY)
                        p.viewSpin.y = kSpinMaxY;
                    else if (p.viewSpin.y < -kSpinMaxY)
                        p.viewSpin.y = -kSpinMaxY;

                    if (p.viewSpin.x > kSpinMaxX)
                        p.viewSpin.x = kSpinMaxX;
                    else if (p.viewSpin.x < -kSpinMaxX)
                        p.viewSpin.x = -kSpinMaxX;

                    if (!Fl::has_timeout(
                            (Fl_Timeout_Handler)_handleViewSpinning_cb, this))
                    {
                        Fl::add_timeout(
                            kSpinTimeout,
                            (Fl_Timeout_Handler)_handleViewSpinning_cb, this);
                    }
                }
                else
                {
                    // x takes dy changes
                    p.viewSpin.x = double(-dy * speed) / viewportSize.h * 2;

                    // while y takes dx changes
                    p.viewSpin.y = double(dx * speed) / viewportSize.w * 2;
                    _updateViewRotation(p.viewSpin);
                }
            }

            return;
        }
        else
        {
            math::Vector2i pos;
            pos.x = p.viewPosMousePress.x + (p.mousePos.x - p.mousePress.x);
            pos.y = p.viewPosMousePress.y + (p.mousePos.y - p.mousePress.y);
            setViewPosAndZoom(pos, p.viewZoom);
        }
    }

    void
    TimelineViewport::_updateViewRotation(const math::Vector2f& spin) noexcept
    {
        TLRENDER_P();

        math::Vector2f rot;
        auto o = p.environmentMapOptions;
        rot.x = o.rotateX + spin.x;
        rot.y = o.rotateY + spin.y;

        if (rot.y > 180.0F)
            rot.y = -180.F - (180.F - rot.y);
        else if (rot.y < -180.0F)
            rot.y = 180.F - (-180.F - rot.y);

        if (rot.x > 90.0F)
        {
            rot.x = 90.F;
            p.mousePress = _getFocus();
        }
        else if (rot.x < -90.0F)
        {
            rot.x = -90.F;
            p.mousePress = _getFocus();
        }

        o.rotateX = rot.x;
        o.rotateY = rot.y;

        setEnvironmentMapOptions(o);
        redrawWindows();
    }

    void TimelineViewport::_updatePixelBar() const noexcept
    {
        TLRENDER_P();

        bool update = _shouldUpdatePixelBar();

        if (update)
        {
            updatePixelBar();
        }
    }

    int TimelineViewport::handle(int event)
    {
        TLRENDER_P();

#ifdef DEBUG_EVENTS
        bool primary = true;
        if (p.ui->uiSecondary && p.ui->uiSecondary->viewport() == this)
            primary = false;

        if (event != FL_MOVE && event != FL_NO_EVENT)
        {
            LOG_INFO("EVENT=" << fl_eventnames[event]);
            LOG_INFO("FOCUS=" << Fl::focus());
            LOG_INFO("BELOWMOUSE? " << (Fl::belowmouse() == this));
        }
#endif

        int ret = Fl_SuperClass::handle(event);
        if ((event == FL_KEYDOWN || event == FL_KEYUP ||
             (event == FL_PUSH && ret == 1)) &&
            Fl::focus() != this)
        {
            return ret;
        }

        p.event_x = Fl::event_x();
        p.event_y = Fl::event_y();

        switch (event)
        {
        case FL_FOCUS:
            return 1;
        case FL_ENTER:
        {
            p.lastEvent = 0;

            // We have children if we are editing a text widget, so we do not
            // grab focus in that case.
            if (!children())
            {
                bool grab_focus = false;
                if (p.ui->uiPrefs->uiPrefsRaiseOnEnter->value())
                    grab_focus = true;

                if (grab_focus)
                    take_focus();
            }
            
#ifdef __APPLE__
            if (p.ui->uiMenuBar && p.ui->uiPrefs->uiPrefsMacOSMenus->value())
                p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
#endif
            _updateCursor();
            _updatePixelBar();
            updateCoords();
            redraw();
            return 1;
            break;
        }
        case FL_LEAVE:
        {
            p.lastEvent = 0;
            const float NaN = std::numeric_limits<float>::quiet_NaN();
            image::Color4f rgba(NaN, NaN, NaN, NaN);
            _updatePixelBar(rgba);
            
            set_cursor(FL_CURSOR_DEFAULT);
            
            redraw(); // to clear the drawing cursor
            return 1;
            break;
        }
        case FL_UNFOCUS:
            return 1;
            break;
        case FL_PUSH:
        {
            if (!children() && Fl::focus() != this && Fl::event_button1())
            {
                take_focus();
                if (Fl::event_clicks() < 2)
                    return 1;
            }
            p.mousePress = _getFocus();
            if (Fl::event_button1())
            {
                if (Fl::event_ctrl())
                {
                    p.viewPosMousePress = p.viewPos;
                    return 1;
                }
                _handlePushLeftMouseButton();
            }
            else if (Fl::event_button2())
            {
                p.viewPosMousePress = p.viewPos;
                if (p.actionMode == ActionMode::kRotate)
                {
                    p.lastEvent = FL_PUSH;
                    return 1;
                }
            }
            else if (Fl::event_button3())
            {
                if (Fl::event_alt())
                {
                    p.last_x = p.event_x;
                    return 1;
                }

                auto time = std::chrono::high_resolution_clock::now();
                p.presentationTime = time;

                set_cursor(FL_CURSOR_DEFAULT);

                Fl_Group::current(0);
                p.popupMenu = new Fl_Menu_Button(0, 0, 0, 0);

                p.popupMenu->textsize(12);
                p.popupMenu->type(Fl_Menu_Button::POPUP3);

                p.ui->uiMain->fill_menu(p.popupMenu);
                p.popupMenu->popup();

                _updateCursor();
                p.popupMenu = nullptr;
            }
            return 1;
        }
        case FL_MOVE:
        {
            updateCoords();
            // If we are drawing or erasing, draw the cursor
            if (p.actionMode != ActionMode::kScrub &&
                p.actionMode != ActionMode::kSelection &&
                p.actionMode != ActionMode::kText &&
                p.actionMode != ActionMode::kRotate)
            {
                redrawWindows();
            }
            if (p.presentation)
            {
                p.presentationTime = std::chrono::high_resolution_clock::now();
            }
            if (p.actionMode != ActionMode::kText)
                _updateCursor();
            _updatePixelBar();
            return 1;
        }
        case FL_RELEASE:
        {
            if (p.actionMode == ActionMode::kPolygon ||
                p.actionMode == ActionMode::kFilledPolygon)
                return 1;
            
            if (p.actionMode == ActionMode::kScrub ||
                p.actionMode == ActionMode::kRotate)
            {
                p.rotDir.x = p.rotDir.y = 0;

                if (p.lastEvent == FL_DRAG &&
                    Fl::event_button() == FL_LEFT_MOUSE && !Fl::event_shift())
                {
                    p.lastEvent = 0;
                    if (!p.player)
                        return 1;
                    p.player->setPlayback(p.playbackMode);
                    panel::redrawThumbnails();
                }
                else
                {
                    if (p.lastEvent == FL_PUSH &&
                        Fl::event_button() == FL_LEFT_MOUSE &&
                        p.actionMode == ActionMode::kScrub &&
                        p.ui->uiPrefs->uiPrefsSingleClickPlayback->value())
                    {
                        p.lastEvent = 0;
                        if (!p.player)
                            return 1;

                        if (_isPlaybackStopped())
                        {
                            setHelpText(_("Play"));
                        }
                        else
                        {
                            setHelpText(_("Stop"));
                        }
                        togglePlayback();
                    }
                }
            }
            else if (
                p.actionMode == ActionMode::kDraw ||
                p.actionMode == ActionMode::kArrow ||
                p.actionMode == ActionMode::kRectangle ||
                p.actionMode == ActionMode::kCircle)
            {
                auto player = getTimelinePlayer();
                if (!player)
                    return 1;

                auto annotation = player->getAnnotation();
                if (p.actionMode != kScrub && !annotation)
                    return 1;

                std::shared_ptr< draw::Shape > s;
                if (annotation)
                    s = annotation->lastShape();
                
                p.lastEvent = 0;

                if (!s->laser)
                    return 1;

                tcp->pushMessage("Laser Fade", 0);

                LaserFadeData* laserData = new LaserFadeData;
                laserData->view = this;
                laserData->annotation = annotation;
                laserData->shape = s;

                Fl::add_timeout(
                    kLaserFadeTimeout, (Fl_Timeout_Handler)laserFade_cb,
                    laserData);
            }
            p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
            _updateCursor();
            return 1;
        }
        case FL_DRAG:
        {
            p.mousePos = _getFocus();
            if (Fl::event_button1())
            {
                if (Fl::event_ctrl())
                {
                    _handleDragMiddleMouseButton();
                }
                else
                {
                    _handleDragLeftMouseButton();
                    _updatePixelBar();
                }
            }
            else if (Fl::event_button2())
            {
                _handleDragMiddleMouseButton();
            }
            else if (Fl::event_button3())
            {
                if (Fl::event_alt())
                {
                    int step = 8;
                    const float speed = _getZoomSpeedValue() / 4.0 / step;

                    int dx = p.last_x - p.event_x;
                    if (abs(dx) < step)
                        return 1;

                    p.last_x = p.event_x;

                    float change = 1.0F;
                    float factor = dx * speed;
                    if (dx > 0)
                    {
                        change += factor;
                        change = 1.0F / change;
                    }
                    else
                    {
                        change -= factor;
                    }

                    float zoom = viewZoom() * change;
                    if (zoom < 0.01F)
                        zoom = 0.01F;
                    else if (zoom > 120.F)
                        zoom = 120.F;

                    setViewZoom(zoom, p.mousePress);
                }
            }
            updateCoords();
            redrawWindows();
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            if (Fl::belowmouse() != this)
                return 0;
            
            float dy = Fl::event_dy();
            const float speed = _getZoomSpeedValue();
            if (_isEnvironmentMap())
            {
                auto o = _p->environmentMapOptions;
                o.focalLength += dy;
                setEnvironmentMapOptions(o);
                redrawWindows();
            }
            else
            {
                if (Fl::event_ctrl())
                {
                    if (!p.player)
                        return 1;
                    _scrub(-dy);
                }
                else
                {
                    float change = 1.0f;
                    float factor = dy * speed;
                    if (dy > 0)
                    {
                        change += factor;
                        change = 1.0f / change;
                    }
                    else
                    {
                        change -= factor;
                    }
                    float zoom = viewZoom() * change;
                    setViewZoom(zoom, _getFocus());
                }
            }
            return 1;
        }
        case FL_KEYBOARD:
        {

            // If we have a text widget, don't swallow key presses
            unsigned rawkey = Fl::event_key();
#if defined(FLTK_USE_WAYLAND)
            if (rawkey >= 'A' && rawkey <= 'Z')
            {
                rawkey = tolower(rawkey);
            }
#endif

            if (kResetChanges.match(rawkey))
            {
                p.ui->uiGamma->value(1.0);
                p.ui->uiGain->value(1.0);
                p.ui->uiSaturation->value(1.0);
                p.ui->uiGammaInput->value(1.0);
                p.ui->uiGainInput->value(1.0);
                p.ui->uiSaturationInput->value(1.0);
                updateDisplayOptions();
                _refresh();
                return 1;
            }
            else if (kExposureMore.match(rawkey))
            {
                p.ui->uiExposureMore->do_callback();
                return 1;
            }
            else if (kExposureLess.match(rawkey))
            {
                p.ui->uiExposureLess->do_callback();
                return 1;
            }
            else if (kGammaMore.match(rawkey))
            {
                float gamma = p.ui->uiGamma->value();
                p.ui->uiGamma->value(gamma + 0.1f);
                p.ui->uiGamma->do_callback();
                return 1;
            }
            else if (kGammaLess.match(rawkey))
            {
                float gamma = p.ui->uiGamma->value();
                gamma -= 0.1F;
                if (gamma < 0.1F)
                    gamma = 0.1F;
                p.ui->uiGamma->value(gamma);
                p.ui->uiGamma->do_callback();
                return 1;
            }
            else if (kSaturationLess.match(rawkey))
            {
                float saturation = p.ui->uiSaturation->value();
                saturation -= 0.1F;
                if (saturation < 0.F)
                    saturation = 0.F;
                p.ui->uiSaturation->value(saturation);
                p.ui->uiSaturation->do_callback();
                return 1;
            }
            else if (kSaturationMore.match(rawkey))
            {
                float saturation = p.ui->uiSaturation->value();
                p.ui->uiSaturation->value(saturation + 0.1F);
                p.ui->uiSaturation->do_callback();
                return 1;
            }
            else if (kToggleEditMode.match(rawkey))
            {
                p.ui->uiEdit->do_callback();
                return 1;
            }
            else if (kToggleToolBar.match(rawkey))
            {
                toggle_action_tool_bar(nullptr, p.ui);
                save_ui_state(p.ui, p.ui->uiToolsGroup);
                return 1;
            }
            else if (kShapeFrameStepFwd.match(rawkey))
            {
                next_annotation_cb(nullptr, p.ui);
                return 1;
            }
            else if (kShapeFrameStepBack.match(rawkey))
            {
                previous_annotation_cb(nullptr, p.ui);
                return 1;
            }
            else if (kFitScreen.match(rawkey))
            {
                frameView();
                return 1;
            }
            else if (kResizeMainWindow.match(rawkey))
            {
                resizeWindow();
                return 1;
            }
            else if (kCenterImage.match(rawkey))
            {
                centerView();
                return 1;
            }
            else if (kTextMode.match(rawkey))
            {
                setActionMode(ActionMode::kText);
                return 1;
            }
            else if (kScrubMode.match(rawkey))
            {
                setActionMode(ActionMode::kScrub);
                return 1;
            }
            else if (kDrawMode.match(rawkey))
            {
                setActionMode(ActionMode::kDraw);
                return 1;
            }
            else if (kEraseMode.match(rawkey))
            {
                setActionMode(ActionMode::kErase);
                return 1;
            }
            else if (kPolygonMode.match(rawkey))
            {
                setActionMode(ActionMode::kPolygon);
                return 1;
            }
            else if (kArrowMode.match(rawkey))
            {
                setActionMode(ActionMode::kArrow);
                return 1;
            }
            else if (kCircleMode.match(rawkey))
            {
                setActionMode(ActionMode::kCircle);
                return 1;
            }
            else if (kRectangleMode.match(rawkey))
            {
                setActionMode(ActionMode::kRectangle);
                return 1;
            }
            else if (
                (p.actionMode != ActionMode::kScrub &&
                 p.actionMode != ActionMode::kSelection &&
                 p.actionMode != ActionMode::kText) &&
                kPenSizeMore.match(rawkey))
            {
                auto settings = p.ui->app->settings();
                std_any value;
                value = settings->getValue<std::any>(kPenSize);
                int v = std_any_empty(value) ? 12 : std_any_cast<int>(value);
                ++v;
                settings->setValue(kPenSize, v);
                redrawWindows();
                return 1;
            }
            else if (
                (p.actionMode != ActionMode::kScrub &&
                 p.actionMode != ActionMode::kSelection &&
                 p.actionMode != ActionMode::kText) &&
                kPenSizeLess.match(rawkey))
            {
                auto settings = p.ui->app->settings();
                std_any value;
                value = settings->getValue<std::any>(kPenSize);
                int v = std_any_empty(value) ? 12 : std_any_cast<int>(value);
                --v;
                settings->setValue(kPenSize, v);
                redrawWindows();
                return 1;
            }
            else if (kAreaMode.match(rawkey))
            {
                setActionMode(ActionMode::kSelection);
                return 1;
            }
            else if (kPlayDirection.match(rawkey))
            {
                togglePlayback();
                return 1;
            }
            else if (kPlayFwd.match(rawkey))
            {
                playForwards();
                return 1;
            }
            else if (kPlayBack.match(rawkey))
            {
                playBackwards();
                return 1;
            }
            else if (kFrameStepFwd.match(rawkey))
            {
                frameNext();
                return 1;
            }
            else if (kFrameStepBack.match(rawkey))
            {
                framePrev();
                return 1;
            }
            else if (kRedoDraw.match(rawkey))
            {
                redo();
                return 1;
            }
            else if (kUndoDraw.match(rawkey))
            {
                undo();
                return 1;
            }
            else if (kZoomIn.match(rawkey))
            {
                setViewZoom(viewZoom() * 2, _getFocus());
                return 1;
            }
            else if (kZoomOut.match(rawkey))
            {
                setViewZoom(viewZoom() * 0.5, _getFocus());
                return 1;
            }
            else if (kSwitchPenColor.match(rawkey))
            {
                flip_pen_color_cb(nullptr, p.ui);
                return 1;
            }
            else if (kPreviousChannel.match(rawkey))
            {
                previous_channel_cb(nullptr, p.ui);
                return 1;
            }
            else if (kNextChannel.match(rawkey))
            {
                next_channel_cb(nullptr, p.ui);
                return 1;
            }
            else if (kPreviousImage.match(rawkey))
            {
                previous_file_cb(nullptr, p.ui);
                return 1;
            }
            else if (kNextImage.match(rawkey))
            {
                next_file_cb(nullptr, p.ui);
                return 1;
            }
            else if (kPreviousImageLimited.match(rawkey))
            {
                previous_file_limited_cb(nullptr, p.ui);
                return 1;
            }
            else if (kNextImageLimited.match(rawkey))
            {
                next_file_limited_cb(nullptr, p.ui);
                return 1;
            }
            else if (
                !Fl::event_state(FL_SHIFT) && !Fl::event_state(FL_ALT) &&
                !Fl::event_state(FL_META) &&
                (rawkey >= kZoomMin.key && rawkey <= kZoomMax.key))
            {
                if (rawkey == kZoomMin.key)
                {
                    viewZoom1To1();
                }
                else
                {
                    float z = (float)(rawkey - kZoomMin.key);
                    if (Fl::event_state(FL_CTRL))
                        z = 1.0f / z;
                    setViewZoom(z, _getFocus());
                }
                return 1;
            }

            bool primary = true;
            if (p.ui->uiSecondary && p.ui->uiSecondary->viewport() == this)
                primary = false;

            if (!p.ui->uiMenuGroup->visible() || !primary)
            {
                ret = p.ui->uiMenuBar->handle(FL_SHORTCUT);
            }

#ifdef _WIN32
            // Swallow Left Alt key presses, as Windows try to focus on the
            // menus.
            if (rawkey == FL_Alt_L)
                return 1;
#endif

            return ret;
            break;
        }
        case FL_DND_ENTER:
        case FL_DND_LEAVE:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:
        {
            return 1;
        }
        case FL_PASTE:
        {
            std::string text;
            if (Fl::event_text())
                text = Fl::event_text();
            dragAndDrop(text);
            return 1;
        }
        default:
            break;
        }

        return ret;
    }

    void TimelineViewport::dragAndDrop(const std::string& text) noexcept
    {
        TLRENDER_P();
        
        std::vector<std::string> loadFiles;
        auto tmpFiles = string::split(text, '\n');

        for (auto file : tmpFiles)
        {
            if (!file::isNetwork(file))
            {
                size_t pos = file.find("://");
                if (pos != std::string::npos)
                    file = file.substr(pos + 3, file.size());

                if (file.empty())
                    continue;

#ifdef __linux__
                // Nautilus returns files with spaces in html ( %20% ) format.
                char* decode = strdup(file.c_str());
                fl_decode_uri(decode);
                file = decode;
                free(decode);
#endif
            }
            
            if (file::isDirectory(file))
            {
                std::vector<std::string> movies, sequences, audios;
                parse_directory(file, movies, sequences, audios);
                loadFiles.insert(loadFiles.end(), movies.begin(), movies.end());
                loadFiles.insert(
                    loadFiles.end(), sequences.begin(), sequences.end());
                loadFiles.insert(loadFiles.end(), audios.begin(), audios.end());
                continue;
            }
            else
            {
                // Google Chrome will shorten out URLs when they are https.
                const file::Path path(file);
                if (!file::isReadable(file) && file::isValidType(path))
                {
                    file = "https://" + file;
                }
                
                loadFiles.push_back(file);
            }
        }

        open_files_cb(loadFiles, p.ui);
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

    void
    TimelineViewport::_clipSelectionArea(math::Vector2i& pos) const noexcept
    {
        TLRENDER_P();

        auto renderSize = getRenderSize();
        if (pos.x < 0)
            pos.x = 0;
        if (pos.y < 0)
            pos.y = 0;

        if (pos.x >= renderSize.w)
            pos.x = renderSize.w - 1;
        if (pos.y >= renderSize.h)
            pos.y = renderSize.h - 1;
    }

} // namespace mrv
