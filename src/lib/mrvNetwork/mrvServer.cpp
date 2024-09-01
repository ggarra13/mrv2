// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvNetwork/mrvServer.h"

namespace
{
    const char* kModule = "server";
}

namespace mrv
{

    Server::Server(const uint16_t port) :
        TCP(),
        serverSocket(port)
    {
        std::string msg =
            tl::string::Format(_("Server started at port {0}.")).arg(port);
        LOG_INFO(msg);

        Poco::Timespan timeout(2000000); // 2 Sec
        reactor.setTimeout(timeout);

        start();
    }

    Server::~Server()
    {
        stop();

        for (auto& t : m_threads)
            if (t->joinable())
                t->join();

        ConnectionHandler::clearHandlers();
        LOG_INFO(_("Server stopped."));
    }

    void Server::start()
    {
        // Create a listening server socket on a specific port
        std::thread* connectionThread = new std::thread(
            [this]
            {
                SocketAcceptor<ConnectionHandler> acceptor(
                    serverSocket, reactor);
                reactor.run(); // will block until stopped
            });

        m_running = true;
        std::thread* disconnectionThread = new std::thread(
            [this]
            {
                while (running())
                {
                    // m_running is volatile so this loop is not optimized
                }
                reactor.stop(); // release reactor.run()
            });
        m_threads.push_back(disconnectionThread);
        m_threads.push_back(connectionThread);
    }

    bool Server::hasReceive() const
    {
        ConnectionHandler* handler = ConnectionHandler::handler();
        if (!handler)
            return false;
        return handler->hasReceive();
    }

    void Server::pushMessage(const Message& message)
    {
        if (m_lock)
            return;
        ConnectionHandler* handler = ConnectionHandler::handler();
        if (!handler)
            return;
        handler->pushMessage(message);
    }

    Message Server::popMessage()
    {
        ConnectionHandler* handler = ConnectionHandler::handler();
        if (!handler)
            throw std::runtime_error(
                "Server::popMessage with nothing on queue.");
        return handler->popMessage();
    }

    void Server::sendMessages()
    {
        ConnectionHandler* handler = ConnectionHandler::handler();
        if (!handler)
            return;
        handler->sendMessages();
    }

    void Server::receiveMessages()
    {
        ConnectionHandler* handler = ConnectionHandler::handler();
        if (!handler)
            return;
        handler->receiveMessages();
    }

} // namespace mrv
