// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/CompareActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/FilesModel.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct CompareActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void CompareActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Next"] = std::make_shared<ui::Action>(
                "Next", "Next", ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextB();
                    }
                });

            p.actions["Prev"] = std::make_shared<ui::Action>(
                "Previous", "Prev", ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevB();
                    }
                });

            const std::array<
                std::string, static_cast<size_t>(timeline::CompareMode::Count)>
                compareIcons = {"CompareA",          "CompareB",
                                "CompareWipe",       "CompareOverlay",
                                "CompareDifference", "CompareHorizontal",
                                "CompareVertical",   "CompareTile"};
            const std::array<
                ui::Key, static_cast<size_t>(timeline::CompareMode::Count)>
                compareShortcuts = {ui::Key::A,       ui::Key::B,
                                    ui::Key::W,       ui::Key::Unknown,
                                    ui::Key::Unknown, ui::Key::Unknown,
                                    ui::Key::Unknown, ui::Key::T};
            const std::array<
                std::string, static_cast<size_t>(timeline::CompareMode::Count)>
                compareToolTips = {
                    string::Format("Show the A file\n"
                                   "\n"
                                   "Shortcut: {0}")
                        .arg(ui::getLabel(
                            compareShortcuts[static_cast<size_t>(
                                timeline::CompareMode::A)],
                            static_cast<int>(ui::KeyModifier::Control))),
                    string::Format("Show the B file\n"
                                   "\n"
                                   "Shortcut: {0}")
                        .arg(ui::getLabel(
                            compareShortcuts[static_cast<size_t>(
                                timeline::CompareMode::B)],
                            static_cast<int>(ui::KeyModifier::Control))),
                    string::Format(
                        "Wipe between the A and B files\n"
                        "\n"
                        "Use the Alt key + left mouse button to move the wipe\n"
                        "\n"
                        "Shortcut: {0}")
                        .arg(ui::getLabel(
                            compareShortcuts[static_cast<size_t>(
                                timeline::CompareMode::Wipe)],
                            static_cast<int>(ui::KeyModifier::Control))),
                    "Show the A file over the B file with transparency",
                    "Show the difference between the A and B files",
                    "Show the A and B files side by side",
                    "Show the A file above the B file",
                    string::Format("Tile the A and B files\n"
                                   "\n"
                                   "Shortcut: {0}")
                        .arg(ui::getLabel(
                            compareShortcuts[static_cast<size_t>(
                                timeline::CompareMode::Tile)],
                            static_cast<int>(ui::KeyModifier::Control))),
                };
            const auto compareEnums = timeline::getCompareModeEnums();
            const auto comapreLabels = timeline::getCompareModeLabels();
            for (size_t i = 0; i < compareEnums.size(); ++i)
            {
                const auto mode = compareEnums[i];
                p.actions[comapreLabels[i]] = std::make_shared<ui::Action>(
                    timeline::getLabel(mode), compareIcons[i],
                    compareShortcuts[i],
                    static_cast<int>(ui::KeyModifier::Control),
                    [appWeak, mode]
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto options =
                                app->getFilesModel()->getCompareOptions();
                            options.mode = mode;
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    });
                p.actions[comapreLabels[i]]->toolTip = compareToolTips[i];
            }

            const auto compareTimeEnums = timeline::getCompareTimeModeEnums();
            const auto comapreTimeLabels = timeline::getCompareTimeModeLabels();
            const std::array<
                std::string, static_cast<size_t>(timeline::CompareMode::Count)>
                compareTimeToolTips = {
                    "Compare relative times", "Compare absolute times"};
            for (size_t i = 0; i < compareTimeEnums.size(); ++i)
            {
                const auto mode = compareTimeEnums[i];
                p.actions[comapreTimeLabels[i]] = std::make_shared<ui::Action>(
                    comapreTimeLabels[i],
                    [appWeak, mode]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getFilesModel()->setCompareTime(mode);
                        }
                    });
                p.actions[comapreTimeLabels[i]]->toolTip =
                    compareTimeToolTips[i];
            }
        }

        CompareActions::CompareActions() :
            _p(new Private)
        {
        }

        CompareActions::~CompareActions() {}

        std::shared_ptr<CompareActions> CompareActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareActions>(new CompareActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        CompareActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
