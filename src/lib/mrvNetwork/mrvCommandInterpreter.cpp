// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvLaserFadeData.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvCommandInterpreter.h"
#include "mrvNetwork/mrvCompareOptions.h"
#include "mrvNetwork/mrvDisplayOptions.h"
#include "mrvNetwork/mrvImageOptions.h"
#include "mrvNetwork/mrvLUTOptions.h"
#include "mrvNetwork/mrvTimelineItemOptions.h"
#include "mrvNetwork/mrvProtocolVersion.h"

#if defined(OPENGL_BACKEND) || defined(BOTH_BACKEND)
#    include "mrvGL/mrvGLUtil.h"
#    include "mrvGL/mrvGLJson.h"
#endif

#if defined(VULKAN_BACKEND)
#    include "mrvVk/mrvVkUtil.h"
#    include "mrvVk/mrvVkJson.h"
#endif

#include "mrvCore/mrvFile.h"

#include <tlCore/StringFormat.h>

#include <FL/Fl_Multiline_Input.H>




namespace
{
    const char* kModule = "inter";
    const double kTimeout = 0.01;
} // namespace

namespace mrv
{

    void matchRemoteImagePosition(
        const math::Vector2i& remoteViewPos, float remoteZoom,
        const math::Size2i& remoteViewport, const math::Size2i& renderSize,
        const math::Size2i& localViewport, math::Vector2i& localViewPos,
        float& localZoom)
    {
        double aspectW =
            localViewport.w / static_cast<double>(remoteViewport.w);
        double aspectH =
            localViewport.h / static_cast<double>(remoteViewport.h);

        localZoom = remoteZoom * std::min(aspectW, aspectH);

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

#ifndef NDEBUG
        std::cerr << "Command: " << message << std::endl;
#endif

        try
        {
            using namespace panel;

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
            else if (c == "Timeline Key Press")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                const int key = message["value"];
                const int modifiers = message["modifiers"];
                ui->uiTimeline->keyPressEvent(key, modifiers);
            }
            else if (c == "Timeline Key Release")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                const int key = message["value"];
                const int modifiers = message["modifiers"];
                ui->uiTimeline->keyReleaseEvent(key, modifiers);
            }
            else if (c == "Timeline Mouse Press")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                const int button = message["button"];
                const bool on = message["on"];
                const int modifiers = message["modifiers"];
                ui->uiTimeline->mousePressEvent(button, on, modifiers);
            }
            else if (c == "Timeline Mouse Move")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                float X = message["X"];
                X *= ui->uiTimeline->pixel_w();
                float Y = message["Y"];
                Y *= ui->uiTimeline->pixel_h();
                ui->uiTimeline->mouseMoveEvent(X, Y);
            }
            else if (c == "Timeline Mouse Release")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                float X = message["X"];
                X *= ui->uiTimeline->pixel_w();
                float Y = message["Y"];
                Y *= ui->uiTimeline->pixel_h();
                int button = message["button"];
                bool on = message["on"];
                int modifiers = message["modifiers"];
                ui->uiTimeline->mouseReleaseEvent(X, Y, button, on, modifiers);
            }
            else if (c == "Timeline Widget Scroll")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                float X = message["X"];
                float Y = message["Y"];
                int modifiers = message["modifiers"];
                ui->uiTimeline->scrollEvent(X, Y, modifiers);
            }
            else if (c == "Timeline Fit")
            {
                bool receive = prefs->ReceiveTimeline->value();
                if (!receive || !player)
                {
                    tcp->unlock();
                    return;
                }
                ui->uiTimeline->frameView();
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
                panel::redrawThumbnails();
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
                TimelineClass* c = ui->uiTimeWindow;
                c->uiVolume->value(value);
                c->uiVolume->redraw();
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
                TimelineClass* c = ui->uiTimeWindow;
                c->uiAudioTracks->value(value);
                c->uiAudioTracks->do_callback();
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
            else if (c == "setOCIOOptions")
            {
                bool receive = prefs->ReceiveColor->value();
                if (!receive || !view)
                {
                    tcp->unlock();
                    return;
                }
                const tl::timeline::OCIOOptions& local = view->getOCIOOptions();
                tl::timeline::OCIOOptions o = message["value"];

                // If we cannot read the config file, keep the local one
                replace_path(o.fileName);
                if (o.fileName.empty() || !file::isReadable(o.fileName))
                {
                    o.fileName = local.fileName;
                    if (o.fileName.empty() || !file::isReadable(o.fileName))
                    {
                        o.fileName = prefs->uiPrefsOCIOConfig->value();
                    }
                }

                int index = ocio::icsIndex(o.input);
                ui->uiICS->value(index);

                std::string mergedView =
                    ocio::combineView(o.display, o.view);
                index = ocio::viewIndex(mergedView);
                ui->uiOCIOView->value(index);

                index = ocio::lookIndex(o.look);
                ui->uiOCIOLook->value(index);

                view->setOCIOOptions(o);
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
            else if (c == "setBackgroundOptions")
            {
                const tl::timeline::BackgroundOptions& o = message["value"];
                view->setBackgroundOptions(o);
                ui->uiMain->fill_menu(ui->uiMenuBar);
            }
            else if (c == "setCompareOptions")
            {
                const tl::timeline::CompareOptions& o = message["value"];
                app->filesModel()->setCompareOptions(o);
            }
            else if (c == "setStereo3DOptions")
            {
                const Stereo3DOptions& o = message["value"];
                app->filesModel()->setStereo3DOptions(o);
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
            else if (c == "Set Stereo Index")
            {
                int value = message["value"];
                app->filesModel()->setStereo(value);
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
            else if (c == "saturation")
            {
                bool receive = prefs->ReceiveColor->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                float value = message["value"];
                ui->uiSaturation->value(value);
                ui->uiSaturation->do_callback();
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
                auto shape = draw::messageToShape(message["value"]);
                annotation->shapes.push_back(shape);

                // Create annotation menus if not there already
                ui->uiMain->fill_menu(ui->uiMenuBar);
                view->updateUndoRedoButtons();
                view->redrawWindows();
            }
            else if (c == "Remove Shape")
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

                int index = message["value"];
                if (index >= 0 && index < annotation->shapes.size())
                {
                    auto shape = annotation->shapes[index];
                    annotation->remove(shape);
                    view->redrawWindows();
                    ui->uiTimeline->redraw();
                }
            }
            else if (c == "Laser Fade")
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
                auto shape = annotation->lastShape();
                if (!shape)
                {
                    tcp->unlock();
                    return;
                }

                // Start laser fading
                LaserFadeData* laserData = new LaserFadeData;
                laserData->view = view;
                laserData->annotation = annotation;
                laserData->shape = shape;

                Fl::add_timeout(
                    0.0, (Fl_Timeout_Handler)MyViewport::laserFade_cb,
                    laserData);

                // Create annotation menus if not there already
                ui->uiMain->fill_menu(ui->uiMenuBar);
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
                auto shape = dynamic_cast< draw::PathShape* >(lastShape.get());
                if (!shape)
                {
                    tcp->unlock();
                    return;
                }
                const draw::Point& value = message["value"];
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
                auto shape = draw::messageToShape(message["value"]);
                annotation->shapes.pop_back();
                annotation->shapes.push_back(shape);
                view->updateUndoRedoButtons();
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
                auto shape = draw::messageToShape(message["value"]);
                annotation->shapes.push_back(shape);
                // Create annotation menus if not there already
                ui->uiMain->fill_menu(ui->uiMenuBar);
                view->updateUndoRedoButtons();
                view->redrawWindows();
            }
            else if (c == "updateVideoCache")
            {
                if (!player)
                {
                    tcp->unlock();
                    return;
                }
                const otime::RationalTime& time = message["value"];
                player->updateVideoCache(time);
            }
            else if (c == "clearCache")
            {
                if (!player)
                {
                    tcp->unlock();
                    return;
                }
                player->clearCache();
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

                const std::vector<draw::Annotation>& tmp = message["value"];
                std::vector< std::shared_ptr<draw::Annotation> > annotations;
                for (const auto& ann : tmp)
                {
                    std::shared_ptr< draw::Annotation > annotation =
                        messageToAnnotation(ann);
                    annotations.push_back(annotation);
                }

                player->setAllAnnotations(annotations);
                ui->uiTimeline->redraw();
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
                const auto& remoteViewport = message["viewport"];
                const auto viewport = view->getViewportSize();
                const auto renderSize = view->getRenderSize();

                // Output values
                math::Vector2i localViewPos; // Local view position (panning)
                float localZoom;             // Local zoom factor

                // Call the function to match the remote image position
                matchRemoteImagePosition(
                    remoteViewPos, remoteZoom, remoteViewport, renderSize,
                    viewport, localViewPos, localZoom);

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
                const math::Box2i& area = message["value"];
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
                if ((!value && panel::onlyOne()) ||
                    (value && !panel::onlyOne()))
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
                if ((!value && annotationsPanel) ||
                    (value && !annotationsPanel))
                    annotations_panel_cb(nullptr, ui);
            }
            else if (c == "Background Panel")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                if ((!value && backgroundPanel) || (value && !backgroundPanel))
                    background_panel_cb(nullptr, ui);
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
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
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
            else if (c == "setEditMode")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                EditMode value = message["value"];
                editMode = value;
                editModeH = message["height"];
                bool presentation = ui->uiView->getPresentationMode();
                if (!presentation)
                    ui->uiView->resizeWindow();

                set_edit_mode_cb(value, ui);
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
            else if (c == "USD Panel")
            {
#ifdef TLRENDER_USD
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                if ((!value && usdPanel) || (value && !usdPanel))
                    usd_panel_cb(nullptr, ui);
#endif
            }
            else if (c == "NDI Panel")
            {
#ifdef TLRENDER_NDI
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                if ((!value && ndiPanel) || (value && !ndiPanel))
                    ndi_panel_cb(nullptr, ui);
#endif
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
#ifdef MRV2_PYBIND11
                bool value = message["value"];
                if ((!value && pythonPanel) || (value && !pythonPanel))
                    python_panel_cb(nullptr, ui);
#endif
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
                if ((!value && vectorscopePanel) ||
                    (value && !vectorscopePanel))
                    vectorscope_panel_cb(nullptr, ui);
            }
            else if (c == "Stereo 3D Panel")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                if ((!value && stereo3DPanel) || (value && !stereo3DPanel))
                    stereo3D_panel_cb(nullptr, ui);
            }
            else if (c == "setTimelineDisplayOptions")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                timelineui::DisplayOptions value = message["value"];
                ui->uiTimeline->setDisplayOptions(value);
            }
            else if (c == "Timeline/FrameView")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                ui->uiTimeline->frameView();
            }
            else if (c == "Timeline/ScrollToCurrentFrame")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                ui->uiTimeline->setScrollToCurrentFrame(value);
            }
            else if (c == "setTimelineEditable")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }
                bool value = message["value"];
                ui->uiTimeline->setEditable(value);
            }
            else if (c == "Clear Frame Annotations")
            {
                annotation_clear_cb(nullptr, ui);
            }
            else if (c == "Clear All Annotations")
            {
                annotation_clear_all_cb(nullptr, ui);
            }
            else if (c == "Create New Timeline")
            {
                create_new_timeline_cb(ui);
            }
            else if (c == "Add Clip to Timeline")
            {
                int Aindex = message["value"];
                add_clip_to_timeline_cb(Aindex, ui);
            }
            else if (c == "Edit/Frame/Cut")
            {
                edit_cut_frame_cb(nullptr, ui);
            }
            else if (c == "Edit/Frame/Copy")
            {
                edit_copy_frame_cb(nullptr, ui);
            }
            else if (c == "Edit/Frame/Paste")
            {
                edit_paste_frame_cb(nullptr, ui);
            }
            else if (c == "Edit/Frame/Insert")
            {
                edit_insert_frame_cb(nullptr, ui);
            }
            else if (c == "Edit/Audio Clip/Insert")
            {
                std::string audioFile = message["value"];
                edit_insert_audio_clip_cb(ui, audioFile);
            }
            else if (c == "Edit/Audio Clip/Remove")
            {
                edit_remove_audio_clip_cb(nullptr, ui);
            }
            else if (c == "Edit/Audio Gap/Insert")
            {
                edit_insert_audio_gap_cb(nullptr, ui);
            }
            else if (c == "Edit/Audio Gap/Remove")
            {
                edit_remove_audio_gap_cb(nullptr, ui);
            }
            else if (c == "Edit/Slice")
            {
                edit_slice_clip_cb(nullptr, ui);
            }
            else if (c == "Edit/Remove")
            {
                edit_remove_clip_cb(nullptr, ui);
            }
            else if (c == "Edit/Undo")
            {
                edit_undo_cb(nullptr, ui);
            }
            else if (c == "Edit/Redo")
            {
                edit_redo_cb(nullptr, ui);
            }
            else if (c == "setFilesPanelOptions")
            {
                bool receive = prefs->ReceiveUI->value();
                if (!receive)
                {
                    tcp->unlock();
                    return;
                }

                const FilesPanelOptions& o = message["value"];
                app->filesModel()->setFilesPanelOptions(o);
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
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what() << " message=" << message);
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
