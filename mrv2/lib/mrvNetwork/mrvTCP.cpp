// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <tlCore/Time.h>

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvTCP.h"

namespace
{
    const char* kModule = "tcp";

#ifdef _WIN32
    const std::string kHostsFile = "C:\\Windows\\System32\\drivers\\etc\\hosts";
#else
    const std::string kHostsFile = "/etc/hosts";
#endif
} // namespace

namespace mrv
{
    std::string ipToHostname(const std::string& ip)
    {
        std::ifstream file(kHostsFile);
        if (!file.is_open())
        {
            return ip;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string host;
            std::string addr;
            iss >> addr >> host;

            // Skip comments or empty lines
            if (addr.empty() || addr[0] == '#')
            {
                continue;
            }

            // Check if IP address matches
            if (addr == ip)
            {
                file.close();
                return host;
            }
        }

        file.close();
        return ip;
    }

    TCP* tcp = nullptr;

    std::mutex TCP::m_receiveMutex;
    std::list< Message > TCP::m_receive;

    TCP::TCP() {}

    TCP::TCP(const std::string& host, int16_t port)
#ifdef MRV2_NETWORK
        :
        m_address(host, port)
#endif
    {
    }

    TCP::~TCP()
    {
        stop();
        close();
    }

    void TCP::stop()
    {
        m_running = false;
        for (auto t : m_threads)
        {
            if (t->joinable())
                t->join();
            delete t;
        }
        m_threads.clear();
    }

    void TCP::close()
    {
#ifdef MRV2_NETWORK
        // Check if the socket is initialized and valid
        if (m_socket.impl()->initialized())
        {
            m_socket.close();
        }
#endif
    }

    void TCP::pushMessage(const Message& message)
    {
        if (m_lock)
            return;
        std::lock_guard lk(m_sendMutex);
        m_send.push_back(message);
    }

    void TCP::pushMessage(const std::string& command, bool value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, int8_t value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, int16_t value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, int32_t value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, int64_t value)
    {
        Message message = {{"command", command}, {"value", value}};
        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, float value)
    {
        Message message = {{"command", command}, {"value", value}};
        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, double value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(const std::string& command, const std::string& value)
    {
        Message message = {{"command", command}, {"value", value}};

        pushMessage(message);
    }

    void TCP::pushMessage(
        const std::string& command, const tl::math::Vector2i& value)
    {
        nlohmann::json j(value);
        Message message = {{"command", command}, {"value", j}};

        pushMessage(message);
    }

    void TCP::pushMessage(
        const std::string& command, const otime::RationalTime& value)
    {
        nlohmann::json j(value);
        Message message = {{"command", command}, {"value", j}};

        pushMessage(message);
    }

    void
    TCP::pushMessage(const std::string& command, const otime::TimeRange& value)
    {
        nlohmann::json j(value);
        Message message = {{"command", command}, {"value", j}};
        pushMessage(message);
    }

    Message TCP::popMessage()
    {
        std::lock_guard lk(m_receiveMutex);
        auto message = m_receive.front();
        m_receive.pop_front();
        return message;
    }

    Message TCP::receiveMessage()
    {
        int len = 0;
        int size = 0;
        int messageLength = 0;
        Message message;
        message["command"] = "***FAILED***";

#ifdef MRV2_NETWORK
        try
        {
            // Read the message length header from the socket
            size = m_socket.receiveBytes(&messageLength, sizeof(messageLength));

            // Convert the message length from network byte order to host
            // byte order
            messageLength = ntohl(messageLength);

            if (messageLength <= 0 || size <= 0)
            {
                return message;
            }

            // Allocate a buffer to hold the received message
            m_buffer.resize(messageLength);
            memset(m_buffer.data(), 0, messageLength);

            // Receive the message into the pre-allocated buffer
            len = 0;
            while (len < messageLength)
            {
                size = m_socket.receiveBytes(
                    m_buffer.data() + len, messageLength - len);
                if (size <= 0)
                {
                    LOG_ERROR("message not complete");
                    return message;
                }
                len += size;
            }
            message = nlohmann::json::from_bson(m_buffer);
        }
        catch (const Poco::Exception& ex)
        {
            // Handle the exception here, which indicates the client disconnect
            // event
            close();
            m_socket = Poco::Net::StreamSocket(m_address);
            Message msg;
            msg["command"] = "Poco::Exception";
            return msg;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("std::exception caught: " << e.what());

            close();
            m_socket = Poco::Net::StreamSocket(m_address);
            Message msg;
            msg["command"] = "std::exception";
            return msg;
        }
#endif
        return message;
    }

} // namespace mrv
