
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

        // subscribe to the "my_topic" topic
        rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "mrv2", strlen("mrv2"));
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
        {
            nng_msg* msg = nullptr;
            rv = nng_recvmsg(sock, &msg, 0);
            if (rv != 0)
            {
                // handle error
                throw std::runtime_error("Could not receivce message.");
            }

            // extract your binary data from the message body
            void* body = nng_msg_body(msg);
            size_t body_size = nng_msg_len(msg);
            if (!body || body_size == 0)
            {
                throw std::runtime_error("Empty message.");
            }

            // handle your binary data here
            char* buf = new char[body_size];
            std::memcpy( buf, body, body_size );

            
            std::cerr << "Received: " << buf << std::endl;
            delete [] buf;
            if(msg)
                nng_msg_free(msg);
        }
    }

} // namespace mrv
