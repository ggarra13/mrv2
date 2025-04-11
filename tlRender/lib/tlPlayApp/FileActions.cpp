// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FileActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/FilesModel.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct FileActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void FileActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Open"] = std::make_shared<ui::Action>(
                "Open", "FileOpen", ui::Key::O,
                static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });
            p.actions["Open"]->toolTip =
                string::Format("Open a file\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Open"]->shortcut,
                        p.actions["Open"]->shortcutModifiers));

            p.actions["OpenSeparateAudio"] = std::make_shared<ui::Action>(
                "Open With Separate Audio", "FileOpenSeparateAudio", ui::Key::O,
                static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openSeparateAudioDialog();
                    }
                });
            p.actions["OpenSeparateAudio"]->toolTip =
                string::Format("Open a file with separate audio\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["OpenSeparateAudio"]->shortcut,
                        p.actions["OpenSeparateAudio"]->shortcutModifiers));

            p.actions["Close"] = std::make_shared<ui::Action>(
                "Close", "FileClose", ui::Key::E,
                static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            p.actions["Close"]->toolTip =
                string::Format("Close the current file\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Close"]->shortcut,
                        p.actions["Close"]->shortcutModifiers));

            p.actions["CloseAll"] = std::make_shared<ui::Action>(
                "Close All", "FileCloseAll", ui::Key::E,
                static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });
            p.actions["CloseAll"]->toolTip =
                string::Format("Close all files\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Close"]->shortcut,
                        p.actions["Close"]->shortcutModifiers));

            p.actions["Reload"] = std::make_shared<ui::Action>(
                "Reload",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->reload();
                    }
                });

            p.actions["Next"] = std::make_shared<ui::Action>(
                "Next", "Next", ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            p.actions["Prev"] = std::make_shared<ui::Action>(
                "Previous", "Prev", ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            p.actions["NextLayer"] = std::make_shared<ui::Action>(
                "Next Layer", "Next", ui::Key::Equal,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextLayer();
                    }
                });

            p.actions["PrevLayer"] = std::make_shared<ui::Action>(
                "Previous Layer", "Prev", ui::Key::Minus,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevLayer();
                    }
                });

            p.actions["Exit"] = std::make_shared<ui::Action>(
                "Exit", ui::Key::Q, static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });
        }

        FileActions::FileActions() :
            _p(new Private)
        {
        }

        FileActions::~FileActions() {}

        std::shared_ptr<FileActions> FileActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FileActions>(new FileActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        FileActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
