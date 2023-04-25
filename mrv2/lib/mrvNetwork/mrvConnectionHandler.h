// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <exception>
#include <iostream>
#include <list>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/NObserver.h>
#include <Poco/Exception.h>

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvMessageBroker.h"

namespace mrv
{
    using namespace Poco;
    using namespace Poco::Net;

    class ConnectionHandler : public TCP
    {
    public:
        /**
         * @Brief Constructor of the Connection Handler
         * @Note Each object is unique to an accepted connection
         * @Param SteamSocket is the socket accepting the connections
         * @See SocketAcceptor
         * http://pocoproject.org/docs/Poco.Net.SocketAcceptor.html
         * @Param SocketReactor is the reacting engine (threaded) which creates
         * notifications about the socket
         */
        ConnectionHandler(StreamSocket& socket, SocketReactor& reactor);

        /**
         * @Brief Destructor
         */
        virtual ~ConnectionHandler();

        /**
         * @Brief Event Handler when Socket becomes Readable, i.e: there is data
         * waiting to be read
         */
        void onSocketReadable(const AutoPtr<ReadableNotification>& pNf);

        /**
         * @Brief Event Handler when Socket was shutdown on the remote/peer side
         */
        void onSocketShutdown(const AutoPtr<ShutdownNotification>& pNf);

        /**
         * @Brief Event Handler when Socket throws an error
         */
        void onSocketError(const AutoPtr<ErrorNotification>& pNf);

        /**
         * @Brief Event Handler when Socket times-out
         */
        void onSocketTimeout(const AutoPtr<TimeoutNotification>& pNf);

        /**
         * @brief Stop the connection and delete itself.
         *
         */
        void stop() override;

        void sendMessages() override;

        void receiveMessages() override;

    public:
        //! Return a conncetion handler or null if not connected yet.
        static ConnectionHandler* handler();

        //! Deletes all connection handles
        static void clearHandlers();

    protected:
        //! Gets the remote (client) IP.
        std::string getIP() const;

        //! Sync client to server data.
        void syncClient();

        //! Sync client to server's UI.
        void syncUI();

        //! Remove a handler from the list.
        void removeHandler(ConnectionHandler* handler);

    protected:
        //! Socket Reactor-Notifier
        SocketReactor& _reactor;

        //! List of connector handlers
        static std::vector< ConnectionHandler* > handlers;

        //! Message publisher to all clients
        static MessageBroker messageBroker;
    };

} // namespace mrv
