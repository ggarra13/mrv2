// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <exception>
#include <iostream>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/NObserver.h>
#include <Poco/Exception.h>

#include <tlCore/StringFormat.h>

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvMessagePublisher.h"
#include "mrvNetwork/mrvConnectionHandler.h"

namespace
{
    const char* kModule = "handler";
}

namespace mrv
{
    using namespace Poco;
    using namespace Poco::Net;

    std::vector< ConnectionHandler* > ConnectionHandler::handlers;
    MessagePublisher ConnectionHandler::messagePublisher;

    ConnectionHandler::ConnectionHandler(
        StreamSocket& socket, SocketReactor& reactor) :
        _reactor(reactor),
        TCP()
    {
        m_socket = socket;

        auto clientIP = getIP();

        messagePublisher.add(clientIP, socket);

        handlers.push_back(this);

        _reactor.addEventHandler(
            m_socket, NObserver<ConnectionHandler, ReadableNotification>(
                          *this, &ConnectionHandler::onSocketReadable));
        _reactor.addEventHandler(
            m_socket, NObserver<ConnectionHandler, ErrorNotification>(
                          *this, &ConnectionHandler::onSocketError));
        _reactor.addEventHandler(
            m_socket, NObserver<ConnectionHandler, TimeoutNotification>(
                          *this, &ConnectionHandler::onSocketTimeout));

        m_running = true;
        const std::string& host = ipToHostname(getIP());
        std::thread* writer = new std::thread(
            [this, host]
            {
                while (running())
                {
                    sendMessages();
                }
            });
        m_threads.push_back(writer);

        //! Log the new connection
        std::string msg =
            tl::string::Format(_("A client connected from {0}")).arg(host);
        LOG_INFO(msg);

        //! Sync this client to the server
        syncClient();
    }

    /**
     * @Brief Destructor
     */
    ConnectionHandler::~ConnectionHandler(){};

    /**
     * @Brief Event Handler when Socket becomes Readable, i.e: there is data
     * waiting to be read
     */
    void ConnectionHandler::onSocketReadable(
        const AutoPtr<ReadableNotification>& pNf)
    {
        StreamSocket socket = pNf->socket();
        if (socket.available() > 0)
        {
            receiveMessages();
        }
        else
        {
            auto clientIP = getIP();
            const auto& host = ipToHostname(clientIP);
            std::string msg =
                tl::string::Format(_("Client at {0} disconnected.")).arg(host);
            LOG_INFO(msg);
            messagePublisher.remove(clientIP);
            stop();
        }
    }

    /**
     * @Brief Event Handler when Socket was shutdown on the remote/peer side
     */
    void ConnectionHandler::onSocketShutdown(
        const AutoPtr<ShutdownNotification>& pNf)
    {
        stop();
    }

    /**
     * @Brief Event Handler when Socket throws an error
     */
    void ConnectionHandler::onSocketError(const AutoPtr<ErrorNotification>& pNf)
    {
        messagePublisher.remove(getIP());
        stop();
    }

    /**
     * @Brief Event Handler when Socket times-out
     */
    void
    ConnectionHandler::onSocketTimeout(const AutoPtr<TimeoutNotification>& pNf)
    {
    }

    void ConnectionHandler::sendMessages()
    {
        try
        {
            std::lock_guard lk(m_sendMutex);
            while (hasSend())
            {
                auto message = m_send.front();
                m_send.pop_front();

                messagePublisher.publish(message);
            }
        }
        catch (Poco::Exception& ex)
        {
            LOG_ERROR("Poco::Exception caught: " << ex.displayText());
            messagePublisher.remove(getIP());
            stop();
        }
    }

    void ConnectionHandler::receiveMessages()
    {
        if (m_socket.available() > 0)
        {
            try
            {
                std::lock_guard lk(m_receiveMutex);
                Message message = receiveMessage();
                m_receive.push_back(message);

                auto clientIP = getIP();
                // Publish message to other subscribers
                messagePublisher.publish(message, clientIP);
            }
            catch (Poco::Exception& ex)
            {
                LOG_ERROR("Poco::Exception caught: " << ex.displayText());
                stop();
            }
        }
    }

    void ConnectionHandler::stop()
    {
        TCP::stop();

        _reactor.removeEventHandler(
            m_socket, NObserver<ConnectionHandler, ReadableNotification>(
                          *this, &ConnectionHandler::onSocketReadable));
        _reactor.removeEventHandler(
            m_socket, NObserver<ConnectionHandler, ErrorNotification>(
                          *this, &ConnectionHandler::onSocketError));
        _reactor.removeEventHandler(
            m_socket, NObserver<ConnectionHandler, TimeoutNotification>(
                          *this, &ConnectionHandler::onSocketTimeout));
        messagePublisher.remove(getIP());
        TCP::close();

        removeHandler(this);
        delete this;
    }

    ConnectionHandler* ConnectionHandler::handler()
    {
        if (handlers.empty())
            return nullptr;
        return handlers.front();
    }

    void ConnectionHandler::removeHandler(ConnectionHandler* handler)
    {
        handlers.erase(
            std::remove(handlers.begin(), handlers.end(), handler),
            handlers.end());
    }

    void ConnectionHandler::clearHandlers()
    {
        for (auto handler : handlers)
        {
            delete handler;
        }
        handlers.clear();
    }

    std::string ConnectionHandler::getIP() const
    {
        Poco::Net::SocketAddress clientAddress = m_socket.peerAddress();
        std::string clientIP = clientAddress.host().toString();
        return clientIP;
    }

} // namespace mrv
