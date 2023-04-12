
#include <exception>
#include <iostream>
#include <cstring>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include "mrvNetwork/mrvClient.h"

namespace mrv
{
    Client::Client(const std::string& host, const unsigned port) :
        TCP()
    {
        char address[128];
        snprintf(address, 128, "tcp://%s:%d", host.c_str(), port);

        std::cerr << "Open client at " << address << std::endl;

        int rv = nng_sub0_open(&sock);
        if (rv != 0)
        {
            throw std::runtime_error("Could not create socket subscription.");
        }

        // subscribe to the "mrv2" topic
        const char* topic = "mrv2";
        size_t topic_len = strlen(topic);
        // rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0);
        // rv = nng_socket_set(sock, NNG_OPT_SUB_SUBSCRIBE, topic, topic_len+1);
        rv = nng_socket_set(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0);
        if (rv != 0)
        {
            // handle error
            throw std::runtime_error("Could not subscribe to mrv2 socket.");
        }

        rv = nng_dial(sock, address, NULL, 0);
        if (rv != 0)
        {
            // handle error
            throw std::runtime_error("Could not dial to socket.");
        }
    }

    void Client::receiveMessage()
    {
        // receive the binary data
        nng_msg* msg;
        int rv;
        if ((rv = nng_recvmsg(sock, &msg, 0)) != 0)
        {
            throw std::runtime_error("Could not receive message.");
        }

        size_t len = nng_msg_len(msg);
        if (len == 0)
        {
            nng_msg_free(msg);
            return;
        }

        std::string message;
        message.resize(len);
        memcpy(message.data(), nng_msg_body(msg), len);
        nng_msg_free(msg);

        m_messages.push_back(message);
    }

} // namespace mrv
