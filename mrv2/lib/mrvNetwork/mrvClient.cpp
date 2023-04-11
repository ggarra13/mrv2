
#include <exception>
#include <iostream>
#include <cstring>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include "mrvNetwork/mrvClient.h"

namespace mrv
{
    Client::Client(const std::string& host, unsigned port)
    {
        char address[128];
        snprintf(address, 128, "tcp://%s:%d", host.c_str(), port);

        nng_socket sock;
        int rv = nng_sub_open(&sock);
        if (rv != 0)
        {
            throw std::runtime_error("Could not create socket subscription.");
        }

        // subscribe to the "mrv2" topic
        const char* topic = "mrv2";
        size_t topic_len = strlen(topic);
        rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, topic, topic_len);
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

        while (1)
        {        // receive the binary data
            char* buf = nullptr;
            size_t body_size = 0;
            rv = nng_recv(sock, &buf, &body_size, NNG_FLAG_ALLOC);
            if (rv != 0)
            {
                // handle error
                throw std::runtime_error("Could not receive message.");
            }

            // handle your binary data here
            std::cerr << "Received: " << buf << std::endl;

            // free the allocated buffer
            nng_free(buf, body_size);
        }
    }

} // namespace mrv
