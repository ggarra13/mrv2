// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Example of a basic pub/sub mechanism
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "mrvNetwork/mrvTCP.h"

#include "mrvFl/mrvIO.h"

namespace mrv
{
    using Poco::Net::Socket;

    typedef std::string ClientIP;
    typedef std::unordered_map<ClientIP, Poco::Net::StreamSocket> IPsToSocket;

    class MessageBroker
    {
    public:
        //! Relay a message from a client to all clients except the one
        //! that sent the original message
        void publish(const Message& message, const ClientIP& client = "");

        void add(const ClientIP& ip, Poco::Net::StreamSocket& socket);

        IPsToSocket::iterator remove(const ClientIP& ip);

    private:
        void shutdown(Poco::Net::StreamSocket& socket);

        IPsToSocket sockets;
    };

} // namespace mrv
