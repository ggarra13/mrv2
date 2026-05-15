// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvSettingsObject.h"

#include "mrvUI/mrvAsk.h"

#include "mrvNetwork/mrvDummyClient.h"
#include "mrvNetwork/mrvWebRTCClient.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvInput.h"

#include "mrvIcons/Network.h"

#include "mrvCore/mrvUtil.h"

#include <FL/Fl_Input.H>

#include "mrvFl/mrvIO.h"

#include "mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "w3tc";
}

namespace mrv
{

#ifdef MRV2_NETWORK
    namespace panel
    {

        struct WebRTCPanel::Private
        {
            Fl_Button* createButton = nullptr;
            Fl_Group* roomGroup = nullptr;
            Input* room = nullptr;
        };

        WebRTCPanel::WebRTCPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("WebRTC");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Network);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete webrtcPanel;
                    webrtcPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        WebRTCPanel::~WebRTCPanel() {}

        void WebRTCPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();

            std_any value;

            Input* i;
            Fl_Button* b;
            
            g->begin();
            
            int X = 80 * g->w() / 270;
            int Y = 20;

            _r->roomGroup = new Fl_Group(g->x(), Y, g->w(), 30);
            auto iW = new Widget< Input >(
                g->x() + X, Y + 5, g->w() - X - 30, 20, _("Room"));
            _r->room = i = iW;
            i->tooltip(_("Room name to enter."));
            iW->callback(
                [=](auto o)
                {
                });
            _r->roomGroup->end();
            
            const char* kButtonLabel = _("Connect");
            auto bW = new Widget<Fl_Button>(g->x(), Y, 30, 20, kButtonLabel);
            b = _r->createButton = bW;
            bW->callback(
                [=](auto t)
                {
                    if (dynamic_cast< DummyClient* >(tcp) == nullptr)
                    {
                        shutdown();
                        return;
                    }
                    
                    bool showMessage = false;
                    std::string roomId = _r->room->value();
                    if (roomId.size() < 6)
                    {
                        showMessage = true;
                        roomId += generateRandomLetters(6);
                    }
                    
                    _r->room->value(roomId.c_str());
                    
                    if (showMessage)
                    {
                        mrv::fl_alert(_("Share the room ID with the persons "
                                        "that will review the session "
                                        "with you.\n\nNote that this feature "
                                        "is only available in the Pro tier."),
                                      nullptr);
                    }
            
                    tcp = new WebRTCClient(roomId);
                    deactivate();
                });
            
            g->end();
        }

        void WebRTCPanel::deactivate()
        {
            const char* kButtonLabel = _("Disconnect");
            _r->createButton->copy_label(kButtonLabel);
            
            _p->ui->uiMain->fill_menu(_p->ui->uiMenuBar);
        }

        void WebRTCPanel::shutdown()
        {
            tcp->stop();
            tcp->close();
            delete tcp;
            tcp = new DummyClient;

            const char* kButtonLabel = _("Connect");
            _r->createButton->label(kButtonLabel);

            _r->roomGroup->activate();
        }

    } // namespace panel
#endif

} // namespace mrv
