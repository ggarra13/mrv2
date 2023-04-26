// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvPreferences.h"

#include "mrViewer.h"

#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvProtocolVersion.h"
#include "mrvNetwork/mrvConnectionHandler.h"

namespace mrv
{
    void ConnectionHandler::syncUI()
    {
        // Sync UI elements
        ViewerUI* ui = App::ui;
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();

        if (!prefs->SendUI->value())
            return;

        bool value = view->getPresentationMode();
        tcp->pushMessage("Presentation", value);

        if (!value)
        {
            value = view->getFullScreenMode();
            tcp->pushMessage("Fullscreen", value);
        }

        tcp->pushMessage(
            "Menu Bar", static_cast<bool>(ui->uiMenuGroup->visible()));
        tcp->pushMessage("Top Bar", static_cast<bool>(ui->uiTopBar->visible()));
        tcp->pushMessage(
            "Pixel Bar", static_cast<bool>(ui->uiPixelBar->visible()));
        tcp->pushMessage(
            "Bottom Bar", static_cast<bool>(ui->uiBottomBar->visible()));
        tcp->pushMessage(
            "Status Bar", static_cast<bool>(ui->uiStatusBar->visible()));
        tcp->pushMessage(
            "Action Bar", static_cast<bool>(ui->uiToolsGroup->visible()));
    }
} // namespace mrv
