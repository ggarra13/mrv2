// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <Poco/Net/TCPServer.h>

namespace mrv
{
    const int kCOMFYUI_PORT_NUMBER = 55121;

    class ComfyUIListener
    {
    public:
        ComfyUIListener(uint16_t port = kCOMFYUI_PORT_NUMBER);
        ~ComfyUIListener();

    public:
        Poco::Net::TCPServer server;
    };
} // namespace mrv
