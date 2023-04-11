
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include <exception>
#include <iostream>
#include <cstring>

#include "mrvNetwork/mrvServer.h"

namespace mrv
{
    Server::Server(const std::string& host, unsigned port)
    {
        char address[128];
        snprintf(address, 128, "tcp://%s:%d", host.c_str(), port);

        nng_socket sock;
        int rv = nng_pub0_open(&sock);
        if (rv != 0)
        {
            throw std::runtime_error("Could not create socket publishing.");
        }

        // // Set the topic for the publisher socket
        // rv = nng_setopt(sock, NNG_OPT_PUB_TOPIC, "mrv2", strlen("mrv2"));
        // if (rv != 0)
        // {
        //     // handle error
        //     throw std::runtime_error("Could not publish to mrv2 socket.");
        // }

        rv = nng_listen(sock, address, NULL, 0);
        if (rv != 0)
        {
            throw std::runtime_error("Could not listen to socket.");
        }

        const char* my_binary_data = "Hello, world!";

        while (1)
        {
            nng_msg* msg = nullptr;
            rv = nng_msg_alloc(&msg, strlen(my_binary_data) +  1);
            if (rv != 0 || !msg)
            {
                // handle error
                throw std::bad_alloc();
            }

            // copy your binary data into the message body
            void* body = nng_msg_body(msg);
            if (body == nullptr)
            {
                throw std::bad_alloc();
            }
            memcpy(body, my_binary_data, strlen(my_binary_data) + 1);

            // publish the message on the "mrv2" topic
            rv = nng_sendmsg(sock, msg, 0);
            if (rv != 0)
            {
                // handle error
                throw std::runtime_error("Could not send message.");
            }

            if(msg)
                nng_msg_free(msg);
        }
    }

} // namespace mrv
