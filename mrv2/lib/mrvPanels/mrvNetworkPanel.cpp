// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <thread>

#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Flex.H>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPack.h"

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvServer.h"
#include "mrvNetwork/mrvClient.h"

#include "mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "net";
}

namespace mrv
{

    struct NetworkPanel::Private
    {
        Fl_Input* host = nullptr;
        Fl_Int_Input* port = nullptr;

        std::thread thread;
        TCP* tcp = nullptr;
    };

    NetworkPanel::NetworkPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {
        add_group("Network");

        // Fl_SVG_Image* svg = load_svg("Network.svg");
        // g->image(svg);

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

        g->clear();

        Fl_Input* i;
        Fl_Int_Input* in;
        Fl_Button* b;
        Fl_Flex* f;

        g->begin();

        int Y = g->y() + 20;
        Fl_Group* gb = new Fl_Group(g->x(), 20, g->w(), 50);
        gb->begin();

        int X = 50 * g->w() / 270;
        _r->host = i =
            new Fl_Input(g->x() + X, Y + 5, g->w() - X, 20, _("Host"));
        i->value("localhost");
        i->tooltip(_("Host or IP to connect to.  For example: 127.0.0.1"));
        i->color((Fl_Color)0xf98a8a800);
        i->textcolor((Fl_Color)56);
        i->labelsize(12);

        _r->port = in = new Fl_Int_Input(g->x() + X, Y + 25, 80, 20, _("Port"));
        in->value("5000");
        in->tooltip(_("Port to connect to.  Make sure your firewall "
                      "allows read/write through it."));
        in->color((Fl_Color)0xf98a8a800);
        in->textcolor((Fl_Color)56);
        in->labelsize(12);
        gb->end();

        f = new Fl_Flex(g->x(), Y + 50, g->w(), 30);
        f->type(Fl_Flex::HORIZONTAL);

        auto bW =
            new Widget<Fl_Button>(g->x() + X, Y + 50, 30, 20, _("Server"));
        b = bW;
        bW->callback(
            [=](auto t)
            {
                std::string host = _r->host->value();
                unsigned port = atoi(_r->port->value());
                try
                {
                    if (!_r->tcp)
                        _r->tcp = new Server(host, port);
                    _r->tcp->sendMessage("Hello");
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            });

        bW = new Widget<Fl_Button>(
            g->x() + g->w() - 30, Y + 50, 30, 20, _("Client"));
        b = bW;
        bW->callback(
            [=](auto t)
            {
                std::string host = _r->host->value();
                unsigned port = atoi(_r->port->value());
                try
                {
                    if (!_r->tcp)
                    {
                        _r->tcp = new Client(host, port);

                        _r->thread = std::thread(
                            [=]
                            {
                                while (1)
                                {
                                    while (!_r->tcp->hasMessages())
                                        _r->tcp->receiveMessage();
                                    while (_r->tcp->hasMessages())
                                        std::cerr << _r->tcp->popMessage()
                                                  << std::endl;
                                }
                            });
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            });

        f->end();

        g->end();
    }

} // namespace mrv
