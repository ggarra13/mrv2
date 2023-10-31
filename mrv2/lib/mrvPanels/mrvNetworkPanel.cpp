// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <thread>

#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Flex.H>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvInput.h"
#include "mrvWidgets/mrvIntInput.h"
#include "mrvWidgets/mrvPack.h"

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvParseHost.h"
#include "mrvNetwork/mrvServer.h"
#include "mrvNetwork/mrvClient.h"
#include "mrvNetwork/mrvDummyClient.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "net";
}

namespace mrv
{

#ifdef MRV2_NETWORK
    namespace panel
    {

        struct NetworkPanel::Private
        {
            Fl_Button* createButton = nullptr;
            Fl_Group* hostGroup = nullptr;
            Input* host = nullptr;
            PopupMenu* hostMenu = nullptr;
            Fl_Group* portGroup = nullptr;
            Fl_Int_Input* port = nullptr;
            PopupMenu* typeMenu = nullptr;
            Type type = Type::Client;
        };

        NetworkPanel::NetworkPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Network");

            Fl_SVG_Image* svg = load_svg("Network.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete networkPanel;
                    networkPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        NetworkPanel::~NetworkPanel() {}

        void NetworkPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();

            std_any value;

            Input* i;
            IntInput* in;
            Fl_Button* b;
            PopupMenu* m;

            g->begin();

            int X = 80 * g->w() / 270;
            int Y = 20;

            Fl_Group* gp = new Fl_Group(g->x(), Y, g->w(), 20);

            Fl_Box* box = new Fl_Box(g->x(), Y, 70, 20, _("Type"));
            box->labelsize(12);
            box->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);

            const char* kLabel = _("Client");
            if (_r->type == Type::Server || dynamic_cast< Server* >(tcp))
            {
                _r->type = Type::Server;
                kLabel = _("Server");
            }

            auto pW =
                new Widget<PopupMenu>(g->x() + X, Y, g->w() - X, 20, kLabel);
            m = _r->typeMenu = pW;
            m->add(_("Client"));
            m->add(_("Server"));
            m->value(static_cast<int>(_r->type));

            gp->end();

            pW->callback(
                [=](auto t)
                {
                    _r->type = static_cast<Type>(t->value());
                    refresh();
                });

            Y += 20;

            _r->hostGroup = new Fl_Group(g->x(), Y, g->w(), 30);
            auto iW = new Widget< Input >(
                g->x() + X, Y + 5, g->w() - X - 30, 20, _("Host"));
            _r->host = i = iW;
            i->tooltip(_("Host name or IP to connect to.  "
                         "For example: 127.0.0.1"));
            iW->callback(
                [=](auto o)
                {
                    std::string port;
                    std::string host = o->value();
                    parse_hostname(host, port);
                    if (!port.empty())
                    {
                        _r->port->value(port.c_str());
                    }
                    o->value(host.c_str());
                });

            if (dynamic_cast< Client* >(tcp))
            {
                Client* client = dynamic_cast< Client* >(tcp);
                const std::string& host = client->host();
                _r->host->value(host.c_str());
            }

            auto hM = new Widget<PopupMenu>(
                g->x() + g->w() - 30, Y + 5, 30, 20, "@2>");
            m = _r->hostMenu = hM;
            m->disable_glyph();
            m->disable_label();
            m->tooltip(_("Previously used Hosts"));
            for (const auto& host : settings->recentHosts())
            {
                m->add(host.c_str());
            }
            hM->callback(
                [=](auto o)
                {
                    const Fl_Menu_Item* item = o->mvalue();
                    _r->host->value(item->label());
                });
            _r->hostGroup->end();

            Y += 30;

            if (_r->type == Type::Server)
            {
                _r->hostGroup->hide();
            }

            _r->portGroup = new Fl_Group(g->x(), Y, g->w(), 30);
            auto inV =
                new Widget<IntInput>(g->x() + X, Y + 5, 80, 20, _("Port"));
            _r->port = in = inV;

            value = settings->getValue<std::any>("TCP/Control/Port");
            std::string portNumber = std_any_empty(value)
                                         ? "55150"
                                         : std_any_cast<std::string>(value);

            in->value(portNumber.c_str());
            in->tooltip(
                _("Port to connect to.  Make sure your firewall and router "
                  "allows read/write through it."));
            inV->callback(
                [=](auto w)
                {
                    std::string value = w->value();
                    settings->setValue("TCP/Control/Port", value);
                });

            _r->portGroup->end();

            Y += 30;

            const char* kButtonLabel = _("Connect");
            if (_r->type == Type::Server)
                kButtonLabel = _("Create");
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

                    uint16_t port = atoi(_r->port->value());

                    try
                    {
                        if (_r->type == Type::Client)
                        {
                            std::string host = _r->host->value();
                            tcp = new Client(host, port);
                            settings->addRecentHost(host);
                        }
                        else
                        {
                            tcp = new Server(port);
                        }
                        deactivate();
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                    }
                });

            g->end();

            if (dynamic_cast< DummyClient* >(tcp) == nullptr)
            {
                deactivate();
            }
        }

        void NetworkPanel::deactivate()
        {
            _r->typeMenu->deactivate();
            _r->port->deactivate();
            _r->hostGroup->deactivate();

            const char* kButtonLabel = _("Disconnect");
            if (dynamic_cast< Server* >(tcp))
                kButtonLabel = _("Shutdown");

            _r->createButton->copy_label(kButtonLabel);
            _p->ui->uiMain->fill_menu(_p->ui->uiMenuBar);
        }

        void NetworkPanel::shutdown()
        {
            tcp->stop();
            tcp->close();
            delete tcp;
            tcp = new DummyClient;

            const char* kButtonLabel = _("Connect");
            if (_r->type == Type::Server)
                kButtonLabel = _("Create");
            _r->createButton->label(kButtonLabel);

            _r->typeMenu->activate();
            _r->port->activate();
            _r->hostGroup->activate();
        }

    } // namespace panel
#endif

} // namespace mrv
