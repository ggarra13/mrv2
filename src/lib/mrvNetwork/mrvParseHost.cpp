// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvParseHost.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvApp.h"
#include "mrViewer.h"

namespace mrv
{
    void parse_hostname(std::string& host, std::string& port)
    {
        size_t pos = host.find("tcp://");
        if (pos != std::string::npos)
        {
            host = host.substr(6, host.size());
        }
        pos = host.find(':');
        if (pos != std::string::npos)
        {
            port = host.substr(pos + 1, host.size());
            host = host.substr(0, pos);
        }
    }

    void store_port(const uint16_t port)
    {
        App* app = App::ui->app;
        auto settings = app->settings();
        char buf[64];
        snprintf(buf, 64, "%d", port);
        settings->setValue("TCP/Control/Port", std::string(buf));
    }
} // namespace mrv
