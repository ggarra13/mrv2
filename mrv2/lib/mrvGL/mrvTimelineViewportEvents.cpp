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

#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvColorSpaces.h"

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
#include "mrvCore/mrvUtil.h"
#include "mrvFl/mrvIO.h"

// #define DEBUG_EVENTS

namespace
{
    const char* kModule = "view";
    const int kCrossSize = 10;
    const float kSpinTimeout = 0.025;
    const float kSpinMaxY = 0.25;
    const float kSpinMaxX = 0.25;
} // namespace

namespace mrv
{

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
            float dx = p.event_x / (float)w();
            float dy = p.event_y / (float)h();
            p.compareOptions.wipeCenter.x = dx;
            p.compareOptions.wipeCenter.y = dy;
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
        if (pos.x < 0)
            pos.x = 0;
        if (pos.y < 0)
            pos.y = 0;
        const auto& renderSize = getRenderSize();
        if (pos.x >= renderSize.w)
            pos.x = renderSize.w - 1;
        if (pos.y >= renderSize.h)
            pos.y = renderSize.h - 1;

        math::BBox2i area = p.selection;
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
                    int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
                    const float speedValues[] = {0.1f, 0.25f, 0.5f};
                    float speed = speedValues[idx];
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

                // auto renderSize = getRenderSize();

                // if (pnt.x < 0 || pnt.y < 0 || pnt.x >= renderSize.w ||
                //     pnt.y >= renderSize.h)
                //     return;

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

                    const auto& renderSize = getRenderSize();
                    const float theta = 45 * M_PI / 180;
                    const int nWidth = 35 * renderSize.w / 1024;

                    double tPointOnLine =
                        nWidth / (2 * (tanf(theta) / 2) * lineLength);
                    Imath::V2d pointOnLine = pnt + -tPointOnLine * lineVector;

                    Imath::V2d normalVector(-lineVector.y, lineVector.x);

                    double tNormal = nWidth / (2 * lineLength);
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
            const imaging::Color4f color(
                r / 255.F, g / 255.F, b / 255.F, alpha);
            fl_font(w->textfont(), w->textsize());

#ifdef USE_OPENGL2
            auto shape = std::make_shared< GL2TextShape >();
#else
            auto shape = std::make_shared< GLTextShape >(p.fontSystem);
#endif

            draw::Point pnt(_getRasterf());

            shape->pts.push_back(pnt); // needed
            annotation->push_back(shape);
            // Calculate offset from corner due to cross and the bottom of
            // the font.
            math::Vector2i offset(
                kCrossSize + 2, kCrossSize + fl_height() - fl_descent());

            shape->text = text;
            shape->color = color;

            float pixels_unit = pixels_per_unit();

#ifdef USE_OPENGL2
            shape->font = w->textfont();
            shape->fontSize = w->textsize() / p.viewZoom * pixels_unit;
            auto pos = math::Vector2i(w->x() + offset.x, w->y() + offset.y);
            pos = _getFocus(pos.x, pos.y);
            pnt = _getRasterf(pos.x, pos.y);
            shape->pts[0].x = pnt.x;
            shape->pts[0].y = pnt.y;
#else
            shape->fontFamily = w->fontFamily;
            shape->fontSize = w->textsize() / p.viewZoom * pixels_unit;

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
                if (pos.x < 0)
                    pos.x = 0;
                if (pos.y < 0)
                    pos.y = 0;
                const auto& renderSize = getRenderSize();
                if (pos.x >= renderSize.w)
                    pos.x = renderSize.w - 1;
                if (pos.y >= renderSize.h)
                    pos.y = renderSize.h - 1;

                math::BBox2i area;
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
                SettingsObject* settingsObject = p.ui->app->settingsObject();
                int fltk_color = p.ui->uiPenColor->color();
                Fl::get_color((Fl_Color)fltk_color, r, g, b);
                float alpha = p.ui->uiPenOpacity->value();
                const imaging::Color4f color(
                    r / 255.F, g / 255.F, b / 255.F, alpha);
                std_any value;
                value = settingsObject->value(kPenSize);
                int pen_size = std_any_cast<int>(value);

                value = settingsObject->value(kSoftBrush);
                int softBrush = std_any_cast<int>(value);

                value = settingsObject->value(kTextFont);
                Fl_Font font = std_any_cast<int>(value);

                p.mousePos = _getFocus();
                draw::Point pnt(_getRasterf());

                auto player = getTimelinePlayer();
                if (!player)
                    return;

                auto annotation = player->getAnnotation();
                bool all_frames = false;
                value = p.ui->app->settingsObject()->value(kAllFrames);
                all_frames = std_any_cast<int>(value);
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
                    shape->pts.push_back(pnt);
                    annotation->push_back(shape);
                    _createAnnotationShape();
                    break;
                }
                case ActionMode::kErase:
                {
                    auto shape = std::make_shared< GLErasePathShape >();
                    shape->pen_size = pen_size;
                    shape->color = color;
                    shape->soft = softBrush;
                    shape->pts.push_back(pnt);
                    annotation->push_back(shape);
                    _createAnnotationShape();
                    break;
                }
                case ActionMode::kArrow:
                {
                    auto shape = std::make_shared< GLArrowShape >();
                    shape->pen_size = pen_size;
                    shape->soft = softBrush;
                    shape->color = color;
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    annotation->push_back(shape);
                    _createAnnotationShape();
                    break;
                }
                case ActionMode::kCircle:
                {
                    auto shape = std::make_shared< GLCircleShape >();
                    shape->pen_size = pen_size;
                    shape->soft = softBrush;
                    shape->color = color;
                    shape->center = _getRaster();
                    shape->radius = 0;

                    annotation->push_back(shape);
                    _createAnnotationShape();
                    break;
                }
                case ActionMode::kRectangle:
                {
                    auto shape = std::make_shared< GLRectangleShape >();
                    shape->pen_size = pen_size;
                    shape->soft = softBrush;
                    shape->color = color;
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    shape->pts.push_back(pnt);
                    annotation->push_back(shape);
                    _createAnnotationShape();
                    break;
                }
                case ActionMode::kText:
                {
                    const auto& viewportSize = getViewportSize();
                    float pct = 1.0; // viewportSize.w / 1024.F;
                    auto w = getMultilineInput();

                    value = settingsObject->value(kFontSize);
                    int font_size = std_any_cast<int>(value);
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
        const float sumX = 0.0001;
        const float sumY = 0.0001;
        bool changed = false;
        if (p.viewSpin.x >= sumX)
        {
            p.viewSpin.x -= sumX;
            changed = true;
        }
        else if (p.viewSpin.x <= -sumX)
        {
            p.viewSpin.x += sumX;
            changed = true;
        }
        else
            p.viewSpin.x = 0.0;

        if (p.viewSpin.y >= sumY)
        {
            p.viewSpin.y -= sumY;
            changed = true;
        }
        else if (p.viewSpin.y <= -sumY)
        {
            p.viewSpin.y += sumY;
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

        cursor(FL_CURSOR_MOVE);
        if (p.actionMode == ActionMode::kRotate ||
            (p.actionMode == ActionMode::kScrub && _isEnvironmentMap()))
        {
            p.lastEvent = FL_DRAG;

            const auto& pos = _getFocus();
            int dx = pos.x - p.mousePress.x;

            if (Fl::event_shift())
            {
                int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
                const float speedValues[] = {0.1f, 0.25f, 0.5f};
                float speed = speedValues[idx];
                auto o = p.environmentMapOptions;
                o.focalLength += dx * speed;
                p.mousePress = pos;
                setEnvironmentMapOptions(o);
            }
            else
            {
                int dy = pos.y - p.mousePress.y;

                if (p.environmentMapOptions.spin)
                {
                    p.viewSpin.x = double(-dy) / 360.0; // x takes dy changes
                    p.viewSpin.y =
                        double(dx) / 360.0; // while y takes dx changes

                    if (p.viewSpin.y > kSpinMaxY)
                        p.viewSpin.y = kSpinMaxY;
                    else if (p.viewSpin.y < -kSpinMaxY)
                        p.viewSpin.y = -kSpinMaxY;

                    if (p.viewSpin.x > kSpinMaxX)
                        p.viewSpin.x = kSpinMaxX;
                    else if (p.viewSpin.x < -kSpinMaxX)
                        p.viewSpin.x = -kSpinMaxX;

                    Fl::add_timeout(
                        kSpinTimeout,
                        (Fl_Timeout_Handler)_handleViewSpinning_cb, this);
                }
                else
                {
                    p.viewSpin.x = double(-dy) / 180.0; // x takes dy changes
                    p.viewSpin.y =
                        double(dx) / 90.0; // while y takes dx changes
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

        if (event != FL_MOVE)
        {
            DBGM0("EVENT=" << fl_eventnames[event]);
            DBGM0("FOCUS=" << Fl::focus());
            DBGM0("BELOWMOUSE? " << (Fl::belowmouse() == this));
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
            // if (Fl::focus() == nullptr)
            if (!children())
                take_focus();
#ifdef __APPLE__
            if (p.ui->uiMenuBar && p.ui->uiPrefs->uiPrefsMacOSMenus->value())
                p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
#endif
            _updateCursor();
            updatePixelBar();
            _updateCoords();
            redraw();
            return 1;
            break;
        }
        case FL_LEAVE:
        {
            cursor(FL_CURSOR_DEFAULT);
            constexpr float NaN = std::numeric_limits<float>::quiet_NaN();
            imaging::Color4f rgba(NaN, NaN, NaN, NaN);
            _updatePixelBar(rgba);
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
            }
            p.mousePress = _getFocus();
            if (Fl::event_button1())
            {
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
                    p.viewPosMousePress = p.mousePress;
                    return 1;
                }

                cursor(FL_CURSOR_DEFAULT);

                Fl_Group::current(0);
                p.popupMenu = new Fl_Menu_Button(0, 0, 0, 0);

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
            _updateCoords();
            // If we are drawing or erasing, draw the cursor
            if (p.actionMode != ActionMode::kScrub &&
                p.actionMode != ActionMode::kSelection &&
                p.actionMode != ActionMode::kText &&
                p.actionMode != ActionMode::kRotate)
            {
                _updateCursor();
                redrawWindows();
            }
            _updatePixelBar();
            return 1;
        }
        case FL_RELEASE:
        {
            if (p.actionMode == ActionMode::kScrub ||
                p.actionMode == ActionMode::kRotate)
            {
                if (p.lastEvent == FL_DRAG)
                {
                    if (filesPanel)
                        filesPanel->redraw();
                    if (comparePanel)
                        comparePanel->redraw();
                    p.lastEvent = 0;
                }
                else
                {
                    if (p.lastEvent == FL_PUSH &&
                        Fl::event_button() == FL_LEFT_MOUSE)
                    {
                        if (p.timelinePlayers.empty())
                            return 0;

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
            _updateCursor();
            return 1;
        }
        case FL_DRAG:
        {
            p.mousePos = _getFocus();
            if (Fl::event_button1())
            {
                _handleDragLeftMouseButton();
                _updatePixelBar();
            }
            else if (Fl::event_button2())
            {
                _handleDragMiddleMouseButton();
            }
            else if (Fl::event_button3())
            {
                if (Fl::event_alt())
                {
                    int diff = p.event_x - p.last_x;
                    if (diff == 0)
                        return 1;
                    int dir = -1;
                    if (diff > 0)
                        dir = 1;
                    if (dir != p.zoomDir)
                    {
                        p.viewPosMousePress.x = p.mousePos.x;
                    }
                    p.zoomDir = dir;
                    p.last_x = p.event_x;
                    int dx = p.mousePos.x - p.viewPosMousePress.x;
                    float factor = dx * viewZoom() / 500.0f;
                    float zoom = viewZoom() + factor;
                    if (zoom < 0.01F)
                        zoom = 0.01F;
                    else if (zoom > 120.F)
                        zoom = 120.F;
                    setViewZoom(zoom, p.mousePos);
                }
            }
            _updateCoords();
            redrawWindows();
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            float dy = Fl::event_dy();
            int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
            const float speedValues[] = {0.1f, 0.25f, 0.5f};
            float speed = speedValues[idx];
            float change = 1.0f;
            if (dy > 0)
            {
                change += dy * speed;
                change = 1.0f / change;
            }
            else
            {
                change -= dy * speed;
            }
            if (_isEnvironmentMap())
            {
                auto o = _p->environmentMapOptions;
                o.focalLength += dy;
                setEnvironmentMapOptions(o);
                redrawWindows();
            }
            else
            {
                setViewZoom(viewZoom() * change, _getFocus());
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
                p.ui->uiGammaInput->value(1.0);
                p.ui->uiGainInput->value(1.0);
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
                p.ui->uiGamma->value(gamma - 0.1f);
                p.ui->uiGamma->do_callback();
                return 1;
            }
            else if (kToggleToolBar.match(rawkey))
            {
                toggle_action_tool_bar(nullptr, p.ui);
                save_ui_state(p.ui, p.ui->uiToolsGroup);
                return 1;
            }
            else if (kToggleEditMode.match(rawkey))
            {
                p.ui->uiEdit->do_callback();
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
                p.actionMode == ActionMode::kDraw && kPenSizeMore.match(rawkey))
            {
                auto settingsObject = p.ui->app->settingsObject();
                std_any value;
                value = settingsObject->value(kPenSize);
                int v = std_any_empty(value) ? 12 : std_any_cast<int>(value);
                ++v;
                settingsObject->setValue(kPenSize, v);
                redrawWindows();
                return 1;
            }
            else if (
                p.actionMode == ActionMode::kDraw && kPenSizeLess.match(rawkey))
            {
                auto settingsObject = p.ui->app->settingsObject();
                std_any value;
                value = settingsObject->value(kPenSize);
                int v = std_any_empty(value) ? 12 : std_any_cast<int>(value);
                --v;
                settingsObject->setValue(kPenSize, v);
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
            else if (kFirstFrame.match(rawkey))
            {
                startFrame();
                return 1;
            }
            else if (kLastFrame.match(rawkey))
            {
                endFrame();
                return 1;
            }
            else if (kRedoDraw.match(rawkey))
            {
                p.ui->uiRedoDraw->do_callback();
                redrawWindows();
                return 1;
            }
            else if (kUndoDraw.match(rawkey))
            {
                p.ui->uiUndoDraw->do_callback();
                redrawWindows();
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
            else if (rawkey >= kZoomMin.key && rawkey <= kZoomMax.key)
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

            int ret = 0;
            if (!p.ui->uiMenuGroup->visible())
            {
                ret = p.ui->uiMenuBar->handle(FL_SHORTCUT);
            }

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

        stringArray tmpFiles, loadFiles;
        mrv::split_string(tmpFiles, text, "\n");

        for (auto file : tmpFiles)
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
            if (mrv::is_directory(file.c_str()))
            {
                stringArray movies, sequences, audios;
                parse_directory(file, movies, sequences, audios);
                loadFiles.insert(loadFiles.end(), movies.begin(), movies.end());
                loadFiles.insert(
                    loadFiles.end(), sequences.begin(), sequences.end());
                loadFiles.insert(loadFiles.end(), audios.begin(), audios.end());
                continue;
            }
            else
            {
                loadFiles.push_back(file);
            }
        }

        open_files_cb(loadFiles, p.ui);
    }

} // namespace mrv
