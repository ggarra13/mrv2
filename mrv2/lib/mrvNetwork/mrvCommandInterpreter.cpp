// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include <FL/Fl_Multiline_Input.H>

#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvIO.h"

#ifdef TLRENDER_GL
#    include "mrvGL/mrvGLUtil.h"
#endif

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvCompareOptions.h"
#include "mrvNetwork/mrvDisplayOptions.h"
#include "mrvNetwork/mrvImageOptions.h"
#include "mrvNetwork/mrvLUTOptions.h"
#include "mrvNetwork/mrvCommandInterpreter.h"
#include "mrvNetwork/mrvProtocolVersion.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "inter";
    const double kTimeout = 0.01;
} // namespace

namespace mrv
{

    void matchRemoteImagePosition(
        const math::Vector2i& remoteViewPos, float remoteZoom,
        const imaging::Size& remoteViewport, const imaging::Size& renderSize,
        const imaging::Size& localViewport, math::Vector2i& localViewPos,
        float& localZoom)
    {
        double aspectW =
            localViewport.w / static_cast<double>(remoteViewport.w);
        double aspectH =
            localViewport.h / static_cast<double>(remoteViewport.h);

        localZoom = remoteZoom * std::min(aspectW, aspectH);
        // std::cerr << "remoteZoon = " << remoteZoom << std::endl;
        // std::cerr << " localZoon = " << localZoom << std::endl;
        // std::cerr << "   aspectW = " << aspectW << std::endl;
        // std::cerr << "   aspectH = " << aspectH << std::endl;

        if (aspectW < aspectH)
        {
            // Scale the remote view position to local viewport
            localViewPos.x = remoteViewPos.x * aspectW;

            double remoteRenderSize = renderSize.h * remoteZoom;
            double localRenderSize = renderSize.h * localZoom;

            localViewPos.y =
                ((remoteViewPos.y * 2 + remoteRenderSize) * aspectH -
                 localRenderSize) /
                2.F;
        }
        else
        {
            // Scale the remote view position to local viewport
            localViewPos.y = remoteViewPos.y * aspectH;

            double remoteRenderSize = renderSize.w * remoteZoom;
            double localRenderSize = renderSize.w * localZoom;

            localViewPos.x =
                ((remoteViewPos.x * 2 + remoteRenderSize) * aspectW -
                 localRenderSize) /
                2.F;
        }
    }

    CommandInterpreter::CommandInterpreter(ViewerUI* gui) :
        ui(gui)
    {
        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    CommandInterpreter::~CommandInterpreter()
    {
        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void CommandInterpreter::parse(const Message& message)
    {
        const std::string& c = message["command"];
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto view = ui->uiView;
        TimelinePlayer* player = nullptr;
        if (view)
            player = view->getTimelinePlayer();

        tcp->lock();

        if (c == "setPlayback")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            timeline::Playback value = message["value"];

            switch (value)
            {
            case timeline::Playback::Forward:
                play_forwards_cb(nullptr, ui);
                break;
            case timeline::Playback::Reverse:
                play_backwards_cb(nullptr, ui);
                break;
            default:
            case timeline::Playback::Stop:
                stop_cb(nullptr, ui);
                break;
            }
        }
        else if (c == "setLoop")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            timeline::Loop value = message["value"];
            player->setLoop(value);
        }
        else if (c == "Open File")
        {
            bool receive = prefs->ReceiveMedia->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            std::string fileName = message["fileName"];
            std::string audioFileName = message["audioFileName"];
            replace_path(fileName);
            if (!audioFileName.empty())
                replace_path(audioFileName);
            app->open(fileName, audioFileName);
        }
        else if (c == "closeAll")
        {
            if (prefs->ReceiveMedia->value())
                close_all_cb(nullptr, ui);
        }
        else if (c == "closeCurrent")
        {
            if (prefs->ReceiveMedia->value())
                close_current_cb(nullptr, ui);
        }
        else if (c == "Media Items")
        {
            bool receive = prefs->ReceiveMedia->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            syncMedia(message);
        }
        else if (c == "seek")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            otime::RationalTime value = message["value"];
            player->seek(value);
        }
        else if (c == "setInOutRange")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            otime::TimeRange value = message["value"];
            player->setInOutRange(value);
        }
        else if (c == "setSpeed")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            double value = message["value"];
            player->setSpeed(value);
        }
        else if (c == "setInPoint")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            player->setInPoint();
        }
        else if (c == "resetInPoint")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->resetInPoint();
        }
        else if (c == "setOutPoint")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->setOutPoint();
        }
        else if (c == "resetInPoint")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->resetOutPoint();
        }
        else if (c == "setVideoLayer")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            int value = message["value"];

            {
                // player->setVideoLayer(value);
                ui->uiColorChannel->value(value);
                ui->uiColorChannel->do_callback();
            }
        }
        else if (c == "Redraw Panel Thumbnails")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            redrawPanelThumbnails();
        }
        else if (c == "setVolume")
        {
            bool receive = prefs->ReceiveAudio->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            float value = message["value"];

            player->setVolume(value);
        }
        else if (c == "setMute")
        {
            bool receive = prefs->ReceiveAudio->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];

            player->setMute(value);
        }
        else if (c == "setAudioOffset")
        {
            bool receive = prefs->ReceiveAudio->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            double value = message["value"];

            player->setAudioOffset(value);
        }
        else if (c == "start")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->start();
        }
        else if (c == "end")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->end();
        }
        else if (c == "framePrev")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->framePrev();
        }
        else if (c == "frameNext")
        {
            bool receive = prefs->ReceiveTimeline->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            player->frameNext();
        }
        else if (c == "undo")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            ui->uiRedoDraw->activate();
            view->undo();
        }
        else if (c == "redo")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            ui->uiUndoDraw->activate();
            view->redo();
        }
        else if (c == "setEnvironmentMapOptions")
        {
            bool receive = prefs->ReceivePanAndZoom->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            const EnvironmentMapOptions& o = message["value"];
            view->setEnvironmentMapOptions(o);
        }
        else if (c == "setColorConfigOptions")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            const tl::timeline::ColorConfigOptions& local =
                view->getColorConfigOptions();
            tl::timeline::ColorConfigOptions o = message["value"];

            // If we cannot read the config file, keep the local one
            replace_path(o.fileName);
            if (o.fileName.empty() || !!is_readable(o.fileName))
            {
                o.fileName = local.fileName;
                if (o.fileName.empty() || !is_readable(o.fileName))
                {
                    o.fileName = prefs->uiPrefsOCIOConfig->value();
                }
            }
            view->setColorConfigOptions(o);
        }
        else if (c == "Display Options")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            const tl::timeline::DisplayOptions& o = message["value"];
            app->setDisplayOptions(o);
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Compare Options")
        {
            const tl::timeline::CompareOptions& o = message["value"];
            app->filesModel()->setCompareOptions(o);
        }
        else if (c == "Set A Index")
        {
            int value = message["value"];
            app->filesModel()->setA(value);
        }
        else if (c == "Set B Indexes")
        {
            std::vector<int> values = message["value"];
            app->filesModel()->clearB();
            for (auto value : values)
            {
                app->filesModel()->setB(value, true);
            }
        }
        else if (c == "Image Options")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            const tl::timeline::ImageOptions& o = message["value"];
            app->setImageOptions(o);
        }
        else if (c == "LUT Options")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            const tl::timeline::LUTOptions& o = message["value"];
            app->setLUTOptions(o);
        }
        else if (c == "gain")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            float value = message["value"];
            ui->uiGain->value(value);
            ui->uiGain->do_callback();
        }
        else if (c == "gamma")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            float value = message["value"];
            ui->uiGamma->value(value);
            ui->uiGamma->do_callback();
        }
        else if (c == "Clear Note Annotation")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            clear_note_annotation_cb(ui);
            if (annotationsPanel)
            {
                annotationsPanel->notes->value("");
            }
        }
        else if (c == "Create Note Annotation")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            const std::string& text = message["value"];
            add_note_annotation_cb(ui, text);
            if (annotationsPanel)
            {
                annotationsPanel->notes->value(text.c_str());
            }
        }
        else if (c == "Create Shape")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            auto annotation = player->getAnnotation();
            if (!annotation)
            {
                tcp->unlock();
                return;
            }
            auto shape = messageToShape(message["value"]);
            annotation->shapes.push_back(shape);

            // Create annotation menus if not there already
            ui->uiMain->fill_menu(ui->uiMenuBar);
            ui->uiUndoDraw->activate();
            view->redrawWindows();
        }
        else if (c == "Add Shape Point")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            auto annotation = player->getAnnotation();
            if (!annotation)
            {
                tcp->unlock();
                return;
            }
            auto lastShape = annotation->lastShape();
            if (!lastShape)
            {
                tcp->unlock();
                return;
            }
            auto shape = dynamic_cast< tl::draw::PathShape* >(lastShape.get());
            if (!shape)
            {
                tcp->unlock();
                return;
            }
            const tl::draw::Point& value = message["value"];
            shape->pts.push_back(value);
            view->redrawWindows();
        }
        else if (c == "Update Shape")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            auto annotation = player->getAnnotation();
            if (!annotation)
            {
                tcp->unlock();
                return;
            }
            auto shape = messageToShape(message["value"]);
            annotation->shapes.pop_back();
            annotation->shapes.push_back(shape);
            ui->uiUndoDraw->activate();
            view->redrawWindows();
        }
        else if (c == "End Shape")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            auto annotation = player->getAnnotation();
            if (!annotation)
            {
                tcp->unlock();
                return;
            }
            auto shape = messageToShape(message["value"]);
            annotation->shapes.push_back(shape);
            // Create annotation menus if not there already
            ui->uiMain->fill_menu(ui->uiMenuBar);
            ui->uiUndoDraw->activate();
            view->redrawWindows();
        }
        else if (c == "Create Annotation")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }
            bool allFrames = message["value"];
            player->createAnnotation(allFrames);
        }
        else if (c == "Annotations")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive || !player)
            {
                tcp->unlock();
                return;
            }

            const std::vector<tl::draw::Annotation>& tmp = message["value"];
            std::vector< std::shared_ptr<tl::draw::Annotation> > annotations;
            for (const auto& ann : tmp)
            {
                std::shared_ptr< tl::draw::Annotation > annotation =
                    messageToAnnotation(ann);
                annotations.push_back(annotation);
            }

            player->setAllAnnotations(annotations);
            TimelineClass* c = ui->uiTimeWindow;
            c->uiTimeline->redraw();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "viewPosAndZoom")
        {
            bool receive = prefs->ReceivePanAndZoom->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }

            float remoteZoom = message["zoom"];

            // When all files are closed, we get an infinite zoom (null),
            if (isinf(remoteZoom))
            {
                tcp->unlock();
                return;
            }

            const math::Vector2i& remoteViewPos = message["viewPos"];
            const imaging::Size& remoteViewport = message["viewport"];
            const imaging::Size& viewport = view->getViewportSize();
            const imaging::Size& renderSize = view->getRenderSize();

            // Output values
            math::Vector2i localViewPos; // Local view position (panning)
            float localZoom;             // Local zoom factor

            // Call the function to match the remote image position
            matchRemoteImagePosition(
                remoteViewPos, remoteZoom, remoteViewport, renderSize, viewport,
                localViewPos, localZoom);

            view->setViewPosAndZoom(localViewPos, localZoom);
        }
        else if (c == "Show Annotations")
        {
            bool receive = prefs->ReceiveAnnotations->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            view->setShowAnnotations(value);
        }
        else if (c == "Menu Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiMenuBar->show();
            }
            else
            {
                ui->uiMenuBar->hide();
            }
            ui->uiRegion->layout();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Top Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiTopBar->show();
            }
            else
            {
                ui->uiTopBar->hide();
            }
            ui->uiRegion->layout();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Pixel Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiPixelBar->show();
            }
            else
            {
                ui->uiPixelBar->hide();
            }
            ui->uiRegion->layout();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Bottom Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiBottomBar->show();
            }
            else
            {
                ui->uiBottomBar->hide();
            }
            ui->uiRegion->layout();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Status Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiStatusBar->show();
            }
            else
            {
                ui->uiStatusBar->hide();
            }
            ui->uiRegion->layout();
        }
        else if (c == "Action Bar")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if (value)
            {
                ui->uiToolsGroup->show();
            }
            else
            {
                ui->uiToolsGroup->hide();
            }
            ui->uiViewGroup->layout();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        else if (c == "Fullscreen")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            view->setFullScreenMode(value);
        }
        else if (c == "Presentation")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            view->setPresentationMode(value);
        }
        else if (c == "Selection Area")
        {
            bool receive = prefs->ReceiveColor->value();
            if (!receive || !view)
            {
                tcp->unlock();
                return;
            }
            const math::BBox2i& area = message["value"];
            view->setSelectionArea(area);
        }
        else if (c == "One Panel Only")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && onePanelOnly()) || (value && !onePanelOnly()))
                toggle_one_panel_only_cb(nullptr, ui);
        }
        else if (c == "Color Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && colorPanel) || (value && !colorPanel))
                color_panel_cb(nullptr, ui);
        }
        else if (c == "Annotations Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && annotationsPanel) || (value && !annotationsPanel))
                annotations_panel_cb(nullptr, ui);
        }
        else if (c == "Color Area Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && colorAreaPanel) || (value && !colorAreaPanel))
                color_area_panel_cb(nullptr, ui);
        }
        else if (c == "Compare Panel")
        {
            bool value = message["value"];
            if ((!value && comparePanel) || (value && !comparePanel))
                compare_panel_cb(nullptr, ui);
        }
        else if (c == "Devices Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && devicesPanel) || (value && !devicesPanel))
                devices_panel_cb(nullptr, ui);
        }
        else if (c == "Environment Map Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && environmentMapPanel) ||
                (value && !environmentMapPanel))
                environment_map_panel_cb(nullptr, ui);
        }
        else if (c == "Files Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && filesPanel) || (value && !filesPanel))
                files_panel_cb(nullptr, ui);
        }
        else if (c == "Histogram Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && histogramPanel) || (value && !histogramPanel))
                histogram_panel_cb(nullptr, ui);
        }
        else if (c == "Media Info Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && imageInfoPanel) || (value && !imageInfoPanel))
                image_info_panel_cb(nullptr, ui);
        }
        else if (c == "Network Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && networkPanel) || (value && !networkPanel))
                network_panel_cb(nullptr, ui);
        }
        // Logs panel is not sent nor received.
        else if (c == "Python Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && pythonPanel) || (value && !pythonPanel))
                python_panel_cb(nullptr, ui);
        }
        else if (c == "Playlist Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && playlistPanel) || (value && !playlistPanel))
                playlist_panel_cb(nullptr, ui);
        }
        else if (c == "Settings Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && settingsPanel) || (value && !settingsPanel))
                settings_panel_cb(nullptr, ui);
        }
        else if (c == "Vectorscope Panel")
        {
            bool receive = prefs->ReceiveUI->value();
            if (!receive)
            {
                tcp->unlock();
                return;
            }
            bool value = message["value"];
            if ((!value && vectorscopePanel) || (value && !vectorscopePanel))
                vectorscope_panel_cb(nullptr, ui);
        }
        else if (c == "Clear Frame Annotations")
        {
            annotation_clear_cb(nullptr, ui);
        }
        else if (c == "Clear All Annotations")
        {
            annotation_clear_all_cb(nullptr, ui);
        }
        else if (c == "Protocol Version")
        {
            int value = message["value"];
            if (value != kProtocolVersion)
            {
                std::string msg =
                    tl::string::Format(
                        _("Server protocol version is {0}.  Client "
                          "protocol version is {1}"))
                        .arg(value)
                        .arg(kProtocolVersion);
                LOG_ERROR(msg);
            }
        }
        else
        {
            // @todo: Unknown command
            std::string err =
                tl::string::Format("Ignored network command {0}.").arg(c);
            LOG_ERROR(err);
        }
        tcp->unlock();
    }

    void CommandInterpreter::timerEvent()
    {

        if (!tcp)
        {
            Fl::repeat_timeout(
                kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
            return;
        }

        while (tcp->hasReceive())
        {
            const Message& message = tcp->popMessage();
            parse(message);
        }
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void CommandInterpreter::timerEvent_cb(void* arg)
    {
        CommandInterpreter* c = static_cast< CommandInterpreter* >(arg);
        c->timerEvent();
    }
} // namespace mrv
