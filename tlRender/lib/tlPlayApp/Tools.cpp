// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>
#include <iostream>

namespace tl
{
    namespace play_app
    {
        TLRENDER_ENUM_IMPL(
            Tool, "Files", "View", "Color", "Info", "Audio", "Devices",
            "Settings", "Messages", "SystemLog");
        TLRENDER_ENUM_SERIALIZE_IMPL(Tool);

        std::string getText(Tool value)
        {
            const std::array<std::string, static_cast<size_t>(Tool::Count)>
                data = {"Files",       "View",     "Color",
                        "Information", "Audio",    "Devices",
                        "Settings",    "Messages", "System Log"};
            return data[static_cast<size_t>(value)];
        }

        std::string getIcon(Tool value)
        {
            const std::array<std::string, static_cast<size_t>(Tool::Count)>
                data = {"Files",   "View",     "Color",    "Info", "Audio",
                        "Devices", "Settings", "Messages", ""};
            return data[static_cast<size_t>(value)];
        }

        ui::Key getShortcut(Tool value)
        {
            const std::array<ui::Key, static_cast<size_t>(Tool::Count)> data = {
                ui::Key::F1, ui::Key::F2, ui::Key::F3, ui::Key::F4, ui::Key::F5,
                ui::Key::F6, ui::Key::F7, ui::Key::F8, ui::Key::F9};
            return data[static_cast<size_t>(value)];
        }

        std::vector<Tool> toolsInToolbar()
        {
            const std::vector<Tool> out{
                Tool::Files, Tool::View,    Tool::Color,    Tool::Info,
                Tool::Audio, Tool::Devices, Tool::Settings, Tool::Messages};
            return out;
        }

        struct ToolsModel::Private
        {
            std::shared_ptr<observer::Value<int> > activeTool;
        };

        void ToolsModel::_init()
        {
            TLRENDER_P();
            p.activeTool = observer::Value<int>::create(-1);
        }

        ToolsModel::ToolsModel() :
            _p(new Private)
        {
        }

        ToolsModel::~ToolsModel() {}

        std::shared_ptr<ToolsModel> ToolsModel::create()
        {
            auto out = std::shared_ptr<ToolsModel>(new ToolsModel);
            out->_init();
            return out;
        }

        int ToolsModel::getActiveTool() const
        {
            return _p->activeTool->get();
        }

        std::shared_ptr<observer::Value<int> >
        ToolsModel::observeActiveTool() const
        {
            return _p->activeTool;
        }

        void ToolsModel::setActiveTool(int value)
        {
            _p->activeTool->setIfChanged(value);
        }
    } // namespace play_app
} // namespace tl
