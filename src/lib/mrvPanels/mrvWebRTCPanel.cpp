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

#include "mrvOS/mrvString.h"
#include "mrvOS/mrvOS.h"

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
            Input* project = nullptr;
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

            _r->roomGroup = new Fl_Group(g->x(), Y, g->w(), 60);

            auto iW = new Widget< Input >(
                g->x() + X, Y + 5, g->w() - X - 30, 20, _("Project"));
            _r->project = i = iW;
            i->value(settings->getValue<std::string>("WebRTC/Project").c_str());
            i->tooltip(_("Project you are working on."));
            iW->callback(
                [=](auto o)
                {
                    settings->setValue("WebRTC/Project",
                                       std::string(o->value()));
                });

            Y += 30;

            iW = new Widget< Input >(
                g->x() + X, Y + 5, g->w() - X - 30, 20, _("Room"));
            _r->room = i = iW;
            i->value(settings->getValue<std::string>("WebRTC/Room").c_str());
            i->tooltip(_("Room name to enter."));
            iW->callback(
                [=](auto o)
                {
                    settings->setValue("WebRTC/Room", std::string(o->value()));
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
                    std::string projectId = _r->project->value();
                    std::string roomId = _r->room->value();

                    if (projectId.empty())
                    {
                        mrv::fl_alert(_("Please enter a Project.\n\n"),
                                      nullptr);
                        return;
                    }

                    if (roomId.empty())
                    {
                        mrv::fl_alert(_("Please enter a unique Room.\n\n"),
                                      nullptr);
                        return;
                    }

                    roomId = projectId + "_" + string::stripWhitespace(roomId);


                    settings->setValue("WebRTC/Project", projectId);
                    settings->setValue("WebRTC/Room", roomId);

                    // Prepend studio name to roomId to keep the connection
                    // "secret".
                    std::string studio = os::sgetenv("MRV2_WEBRTC_STUDIO");
                    if (studio.empty())
                        studio = p.ui->uiPrefs->uiPrefsWebRTCStudio->value();

                    if (!mrv::app::soporta_voice)
                    {
                        mrv::fl_alert(_("This feature is unlimited on the "
                                        "Pro and Pro+ tiers.\n\n"
                                        "On other tiers, it is limited to 2\n"
                                        "connections and 30 minutes of use."),
                                      nullptr);
                    }

                    tcp = new WebRTCClient(studio, roomId);

                    deactivate();
                });

            g->end();

            if (dynamic_cast< DummyClient* >(tcp) == nullptr)
            {
                deactivate();
            }
        }

        void WebRTCPanel::deactivate()
        {
            const char* kButtonLabel = _("Disconnect");
            _r->createButton->copy_label(kButtonLabel);

            _r->roomGroup->deactivate();

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
