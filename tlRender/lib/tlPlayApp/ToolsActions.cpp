// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/Tools.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void ToolsActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            const auto enums = getToolEnums();
            const auto labels = getToolLabels();
            const std::array<std::string, static_cast<size_t>(Tool::Count)>
                toolTips = {
                    "Show the files tool\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the color tool\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the information tool\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the audio tool\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the devices tool\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the settings\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the messages\n"
                    "\n"
                    "Shortcut: {0}",
                    "Show the system log\n"
                    "\n"
                    "Shortcut: {0}"};
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto tool = enums[i];
                p.actions[labels[i]] = std::make_shared<ui::Action>(
                    getText(tool), getIcon(tool), getShortcut(tool), 0,
                    [this, appWeak, tool](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto toolsModel = app->getToolsModel();
                            const int active = toolsModel->getActiveTool();
                            toolsModel->setActiveTool(
                                static_cast<int>(tool) != active
                                    ? static_cast<int>(tool)
                                    : -1);
                        }
                    });
                if (!toolTips[i].empty())
                {
                    p.actions[labels[i]]->toolTip =
                        string::Format(toolTips[i])
                            .arg(ui::getLabel(
                                p.actions[labels[i]]->shortcut,
                                p.actions[labels[i]]->shortcutModifiers));
                }
            }
        }

        ToolsActions::ToolsActions() :
            _p(new Private)
        {
        }

        ToolsActions::~ToolsActions() {}

        std::shared_ptr<ToolsActions> ToolsActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ToolsActions>(new ToolsActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        ToolsActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
