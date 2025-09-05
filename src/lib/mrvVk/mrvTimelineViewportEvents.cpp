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
#include "mrvFl/mrvLaserFadeData.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvWidgets/mrvHorSlider.h"

#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvVk/mrvVkShape.h"

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

    namespace vulkan
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
                        p.showAnnotations = true;
                    
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
                        auto shape = dynamic_cast< VKRectangleShape* >(s.get());
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
                        auto shape = dynamic_cast< VKPathShape* >(s.get());
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
                        auto shape = dynamic_cast< VKPathShape* >(s.get());
                        if (!shape)
                            return;

                        shape->pts.push_back(pnt);
                        _addAnnotationShapePoint();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kErase:
                    {
                        auto shape = dynamic_cast< VKErasePathShape* >(s.get());
                        if (!shape)
                            return;

                        shape->pts.push_back(pnt);
                        _addAnnotationShapePoint();
                        redrawWindows();
                        return;
                    }
                    case ActionMode::kArrow:
                    {
                        auto shape = dynamic_cast< VKArrowShape* >(s.get());
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
                        auto shape = dynamic_cast< VKCircleShape* >(s.get());
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
                        auto w = getMultilineInput();
                        if (w)
                        {
                            auto pos = _getRasterf();
                            w->pts[0] = pos;
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

        std::shared_ptr<VKTextShape> TimelineViewport::getMultilineInput() const noexcept
        {
            return _p->multilineText;
        }

        int TimelineViewport::acceptMultilineInput() noexcept
        {
            TLRENDER_P();

            Fl::event_dispatch(nullptr);
                        
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

            auto shape = getMultilineInput();
            if (!shape)
                return 0;

            shape->editing = false;

            p.multilineText.reset();
            
            const float pixels_unit = pixels_per_unit();            
            _endAnnotationShape();
            p.ui->uiUndoDraw->activate();

            // Update the windows properly for NDI
            redrawWindows();
            Fl::flush();
            redrawWindows();
            
            take_focus();
            return 1;
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
                    int font = settings->getValue<int>(kTextFont);

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
                        auto shape = std::make_shared< VKPathShape >();
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
                        auto shape = std::make_shared< VKErasePathShape >();
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
                        auto shape = std::make_shared< VKArrowShape >();
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
                        std::shared_ptr<VKFilledPolygonShape> shape;
                        if (p.lastEvent != FL_DRAG)
                        {
                            shape = std::make_shared< VKFilledPolygonShape >();
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
                                std::dynamic_pointer_cast<VKFilledPolygonShape>(s);
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
                        std::shared_ptr<VKPolygonShape> shape;
                        if (p.lastEvent != FL_DRAG)
                        {
                            shape = std::make_shared< VKPolygonShape >();
                            shape->pen_size = pen_size;
                            shape->soft = false;
                            shape->color = color;
                            shape->laser = laser;
                            p.lastEvent = FL_PUSH;
                        }
                        else
                        {
                            auto s = annotation->lastShape();
                            shape = std::dynamic_pointer_cast<VKPolygonShape>(s);
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
                        auto shape = std::make_shared< VKFilledCircleShape >();
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
                        auto shape = std::make_shared< VKCircleShape >();
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
                        auto shape = std::make_shared< VKFilledRectangleShape >();
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
                        auto shape = std::make_shared< VKRectangleShape >();
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
                        auto shape = getMultilineInput();
                        
                        double pixels_unit = pixels_per_unit();
                        double pct = renderSize.h / 1024.F;
                        if (pct < 1.0) pct = 1.0;
                        int font_size = settings->getValue<int>(kFontSize);
                        double fontSize = font_size * pct / pixels_unit;
                        math::Vector2f pos(_getRasterf());
                        if (shape)
                        {
                            shape->pts[0] = pos;
                            shape->fontSize = fontSize;
                            redrawWindows();
                            return;
                        }

                        const std::vector<fs::path>& fontList =
                            image::discoverSystemFonts();
                        p.multilineText = std::make_shared<VKTextShape>();
                        shape = p.multilineText;
  
                        shape->fontSystem = p.fontSystem;
                        shape->fontPath = fontList[(unsigned)font].u8string();
                        shape->fontSize = fontSize;
                        
                        shape->pts.push_back(pos);
                        shape->editing = true;
                        shape->color = color;

                        annotation->push_back(shape);
                        take_focus();

            
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

            auto shape = dynamic_cast<VKTextShape*>(s.get());
            if (!shape)
                return;

            shape->editing = true;
            p.multilineText = std::dynamic_pointer_cast<VKTextShape>(s);

            tcp->pushMessage("Remove Shape", index);

            redrawWindows();
        }

    } // namespace vulkan
    
} // namespace mrv
