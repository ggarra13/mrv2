// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/ServerSocket.h>

#include "mrvNetwork/mrvConnectionHandler.h"
#include "mrvNetwork/mrvMessageBroker.h"
#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class Server : public TCP
    {
    public:
        Server(const uint16_t port = 58000);
        virtual ~Server();

        void start();

        bool hasReceive() const override;

        void pushMessage(const Message& message) override;
        Message popMessage() override;

    protected:
        void sendMessages() override;
        void receiveMessages() override;

    private:
        Poco::Net::ServerSocket serverSocket;
        Poco::Net::SocketReactor reactor;
    };
} // namespace mrv
