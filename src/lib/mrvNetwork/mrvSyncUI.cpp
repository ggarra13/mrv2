// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvPreferences.h"

#include "mrViewer.h"

#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvProtocolVersion.h"
#include "mrvNetwork/mrvTCP.h"

#include <tlTimelineUI/IItem.h>

namespace mrv
{
    void TCP::syncUI(const std::string& peerId)
    {
        // Sync UI elements
        ViewerUI* ui = App::ui;
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();

        if (!prefs->SendUI->value())
            return;

        Message msg;

        bool value = view->getPresentationMode();

        msg["command"] = "Presentation";
        msg["value"] = value;
        tcp->pushToPeer(peerId, msg);

        if (!value)
        {
            value = view->getFullScreenMode();

            msg["command"] = "Fullscreen";
            msg["value"] = value;
            tcp->pushToPeer(peerId, msg);
        }

        msg["command"] = "Menu Bar";
        msg["value"] = static_cast<bool>(ui->uiMenuGroup->visible());
        tcp->pushToPeer(peerId, msg);

        msg["command"] = "Top Bar";
        msg["value"] = static_cast<bool>(ui->uiTopBar->visible());
        tcp->pushToPeer(peerId, msg);

        msg["command"] = "Pixel Bar";
        msg["value"] = static_cast<bool>(ui->uiPixelBar->visible());
        tcp->pushToPeer(peerId, msg);

        msg["command"] = "Bottom Bar";
        msg["value"] = static_cast<bool>(ui->uiBottomBar->visible());
        tcp->pushToPeer(peerId, msg);

        msg["command"] = "Status Bar";
        msg["value"] = static_cast<bool>(ui->uiStatusBar->visible());
        tcp->pushToPeer(peerId, msg);

        msg["command"] = "Action Bar";
        msg["value"] = static_cast<bool>(ui->uiToolsGroup->visible());
        tcp->pushToPeer(peerId, msg);

        // auto options = ui->uiTimeline->getItemOptions();
        // msg["command"] = "setTimelineItemOptions";
        // msg["value"] = options;
        // tcp->pushToPeer(peerId, msg);

        bool editable = ui->uiTimeline->isEditable();
        msg["command"] = "setTimelineEditable";
        msg["value"] = editable;
        tcp->pushToPeer(peerId, msg);
    }
} // namespace mrv
