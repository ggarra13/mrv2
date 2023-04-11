
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

        rv = nng_listen(sock, address, NULL, 0);
        if (rv != 0)
        {
            throw std::runtime_error("Could not listen to socket.");
        }

        std::string message = "Hello, world!";

        while (1)
        {
#if 1
            // create a new NNG message
            nng_msg* msg;
            int rv = nng_msg_alloc(&msg, 0);
            if (rv != 0)
            {
                // handle error
                throw std::runtime_error("Could not allocate message.");
            }

            // set the message body to the binary data
            rv = nng_msg_append(msg, message.c_str(), message.length()); // exclude null terminator
            if (rv != 0)
            {
                // handle error
                nng_msg_free(msg);
                throw std::runtime_error("Could not set message body.");
            }

            // publish the message on the "mrv2" topic
            const char* topic = "mrv2";
            rv = nng_msg_header_append(msg, topic, strlen(topic) + 1); // +1 for null terminator
            if (rv != 0)
            {
                // handle error
                nng_msg_free(msg);
                throw std::runtime_error("Could not set message topic.");
            }

            // send the message
            rv = nng_sendmsg(sock, msg, 0);
            if (rv != 0)
            {
                // handle error
                nng_msg_free(msg);
                throw std::runtime_error("Could not send message.");
            }

            // free the message
            nng_msg_free(msg);
#else
            // publish the binary data on the "mrv2" topic
            rv = nng_send(sock, (void*)my_binary_data, strlen(my_binary_data) + 1, 0);
            if (rv != 0)
            {
                // handle error
                throw std::runtime_error("Could not send message.");
            }
#endif
        }
    }

} // namespace mrv
