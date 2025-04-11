// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace play_app
    {
        //! Tools.
        //!
        //! \todo Add a color picker.
        //! \todo Add a magnifier.
        enum class Tool {
            Files,
            View,
            Color,
            Info,
            Audio,
            Devices,
            Settings,
            Messages,
            SystemLog,

            Count,
            First = Audio
        };
        TLRENDER_ENUM(Tool);
        TLRENDER_ENUM_SERIALIZE(Tool);

        //! Get the tool text.
        std::string getText(Tool);

        //! Get the tool icon.
        std::string getIcon(Tool);

        //! Get the tool keyboard shortcut.
        ui::Key getShortcut(Tool);

        //! Get the tools in the toolbar.
        std::vector<Tool> toolsInToolbar();

        //! Tools model.
        class ToolsModel : public std::enable_shared_from_this<ToolsModel>
        {
            TLRENDER_NON_COPYABLE(ToolsModel);

        protected:
            void _init();

            ToolsModel();

        public:
            ~ToolsModel();

            //! Create a new model.
            static std::shared_ptr<ToolsModel> create();

            //! Get the active tool.
            int getActiveTool() const;

            //! Observe the active tool.
            std::shared_ptr<observer::Value<int> > observeActiveTool() const;

            //! Set the active tool.
            void setActiveTool(int);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
