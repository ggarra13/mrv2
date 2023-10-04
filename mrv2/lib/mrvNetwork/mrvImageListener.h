// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/TCPServer.h>

namespace mrv
{
    class App;

    const int kPORT_NUMBER = 55120;

    class ImageSender
    {
    public:
        ImageSender(uint16_t port = kPORT_NUMBER);

        bool isRunning();
        void sendImage(const std::string& imageData);

    private:
        Poco::Net::SocketAddress address;
        Poco::Net::StreamSocket socket;
    };

    class ImageListener
    {
    public:
        ImageListener(App* app, uint16_t port = kPORT_NUMBER);
        ~ImageListener();

    public:
        Poco::Net::TCPServer server;
    };
} // namespace mrv
