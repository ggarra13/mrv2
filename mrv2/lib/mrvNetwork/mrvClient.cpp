// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <exception>
#include <iostream>

#include <tlCore/StringFormat.h>

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Timespan.h>

#include "mrvFl/mrvIO.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvClient.h"
#include "mrvNetwork/mrvDummyClient.h"

namespace
{
    const char* kModule = "client";
}

namespace mrv
{
    static void clear_tcp_cb(void* data)
    {
        delete tcp;
        tcp = new DummyClient();
        if (networkPanel)
            networkPanel->refresh();
    }

    Client::Client(const std::string& host, const uint16_t port) :
        TCP(host, port)
    {

        try
        {
            m_socket.connect(m_address); // Connect to the server

            std::string msg =
                tl::string::Format(_("Connected to server at {0}, port {1}"))
                    .arg(host)
                    .arg(port);
            LOG_INFO(msg);

            // Set a send/receive timeout of 2 seconds
            Poco::Timespan timeout(2, 0); // 2 Sec
            m_socket.setSendTimeout(timeout);
            m_socket.setReceiveTimeout(timeout);

            m_host = host;
            m_running = true;

            std::thread* receive = new std::thread(
                [this]
                {
                    while (m_running)
                    {
                        receiveMessages();
                    }
                });

            std::thread* send = new std::thread(
                [this]
                {
                    while (m_running)
                    {
                        sendMessages();
                    }
                });

            m_threads.push_back(send);
            m_threads.push_back(receive);
        }
        catch (const Poco::Exception& e)
        {
            throw std::runtime_error(e.displayText());
        }
    }

    Client::~Client() {}

    void Client::sendMessages()
    {
        Message message;
        try
        {
            std::lock_guard lk(m_sendMutex);
            int size;

            while (hasSend())
            {
                message = m_send.front();
                m_send.pop_front();

                std::vector< uint8_t > v_bson =
                    nlohmann::json::to_bson(message);
                int messageLength = v_bson.size();
                if (messageLength == 0)
                    continue;

                // Send command
                int messageLengthHtoNL = htonl(messageLength);

                size = m_socket.sendBytes(
                    &messageLengthHtoNL, sizeof(messageLength));
                if (size <= 0)
                {
                    m_send.clear();
                    m_running = false;
                    close();
                    break;
                }

                int len = 0;
                while (len < messageLength)
                {
                    size = m_socket.sendBytes(
                        v_bson.data() + len, messageLength - len);
                    if (size <= 0)
                    {
                        m_send.clear();
                        m_running = false;
                        close();
                        break;
                    }
                    len += size;
                }
            }
        }
        catch (const Poco::Exception& ex)
        {
            // Handle the exception here, which indicates the client disconnect
            // event
            LOG_INFO(_("Server connection lost."));
            std::lock_guard lk(m_sendMutex);
            m_send.clear();
            m_running = false;
            close();
            Fl::add_timeout(0.005, (Fl_Timeout_Handler)clear_tcp_cb, nullptr);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(_("Exception caught: ") << e.what());
        }
    }

    void Client::receiveMessages()
    {
        if (m_socket.available() > 0)
        {
            std::lock_guard lk(m_receiveMutex);
            Message message = receiveMessage();
            m_receive.push_back(message);
        }
    }
} // namespace mrv
