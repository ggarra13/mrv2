// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "player-qtquick.h"

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <QQmlComponent>
#include <QQmlContext>

namespace tl
{
    namespace examples
    {
        namespace player_qtquick
        {
            App::App(
                int& argc, char** argv,
                const std::shared_ptr<system::Context>& context) :
                QGuiApplication(argc, argv)
            {
                BaseApp::_init(
                    app::convert(argc, argv), context, "player-qtquick",
                    "Example Qt Quick player application.",
                    {app::CmdLineValueArg<std::string>::create(
                        _input, "input", "The input timeline.")});
                const int exitCode = getExit();
                if (exitCode != 0)
                {
                    exit(exitCode);
                    return;
                }

                // Initialize Qt.
                setOrganizationName("tlRender");
                setApplicationName("player-qtquick");

                // Create models and objects.
                _contextObject.reset(new qt::ContextObject(context, this));
                _timeUnitsModel = timeline::TimeUnitsModel::create(context);
                _timeObject.reset(new qt::TimeObject(_timeUnitsModel, this));

                // Open the input file.
                auto timeline = timeline::Timeline::create(_input, context);
                auto player = timeline::Player::create(timeline, context);
                _timelinePlayer.reset(new qt::TimelinePlayer(player, context));

                // Load the QML.
                _qmlEngine.reset(new QQmlApplicationEngine);
                _qmlEngine->rootContext()->setContextProperty(
                    "timelinePlayer", _timelinePlayer.get());
                QQmlComponent component(
                    _qmlEngine.get(),
                    QUrl(QStringLiteral("qrc:/player-qtquick.qml")));
                if (component.status() != QQmlComponent::Status::Ready)
                {
                    throw std::runtime_error(
                        component.errorString().toUtf8().data());
                }
                _qmlObject.reset(component.create());

                // Start playback.
                _timelinePlayer->setPlayback(timeline::Playback::Forward);
            }

            App::~App() {}
        } // namespace player_qtquick
    } // namespace examples
} // namespace tl
