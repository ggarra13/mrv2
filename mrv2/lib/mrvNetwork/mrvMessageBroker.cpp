// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/NetException.h>

#include <tlCore/StringFormat.h>

#include "mrvNetwork/mrvMessageBroker.h"

namespace
{
    const char* kModule = "publisher";
}

namespace mrv
{

    //! Relay a message from a client to all clients except the one
    //! that sent the original message
    void
    MessageBroker::publish(const Message& message, const ClientIP& clientIP)
    {
        int size;
        auto it = sockets.begin();
        while (it != sockets.end())
        {
            if (it->first == clientIP)
            {
                ++it;
                continue;
            }

            Poco::Net::StreamSocket& m_socket = it->second;

            try
            {
                std::vector< uint8_t > v_bson =
                    nlohmann::json::to_bson(message);

                int messageLength = v_bson.size();
                if (messageLength <= 0)
                {
                    continue;
                }

                // Send command
                int messageLengthHtoNL = htonl(messageLength);

                size = m_socket.sendBytes(
                    &messageLengthHtoNL, sizeof(messageLength));
                if (size <= 0)
                {
                    it = sockets.erase(it);
                    continue;
                }

                int len = 0;
                while (len < messageLength)
                {
                    size = m_socket.sendBytes(
                        v_bson.data() + len, messageLength - len);
                    if (size <= 0)
                    {
                        continue;
                    }
                    len += size;
                }
            }
            catch (const Poco::Exception& ex)
            {
                // Handle the exception here, which indicates the client
                // disconnect event
                LOG_ERROR("Poco::Exception caught: " << ex.displayText());
                it = remove(it->first);
                continue;
            }
            catch (const std::exception& e)
            {
                // Handle the exception here, which indicates the client
                // disconnect event
                LOG_ERROR("std::exception caught: " << e.what());
                it = remove(it->first);
                continue;
            }

            ++it;
        }
    }

    void MessageBroker::add(const ClientIP& ip, Poco::Net::StreamSocket& socket)
    {
        sockets[ip] = socket;

        Poco::Timespan timeout(2, 0); // 2 Sec
        socket.setSendTimeout(timeout);
    }

    IPsToSocket::iterator MessageBroker::remove(const ClientIP& ip)
    {
        auto it = sockets.find(ip);
        if (it != sockets.end())
        {
            std::string msg =
                tl::string::Format(_("Removing {0} from message publisher."))
                    .arg(ipToHostname(ip));
            it = sockets.erase(it);
        }
        return it;
    }
} // namespace mrv
