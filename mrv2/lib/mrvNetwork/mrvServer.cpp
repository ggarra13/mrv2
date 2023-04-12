

#include <exception>
#include <iostream>
#include <cstring>

#include "mrvNetwork/mrvServer.h"

#include <nng/protocol/pubsub0/pub.h>

namespace mrv
{
    Server::Server(const std::string& host, unsigned port)
    {
        char address[128];
        snprintf(address, 128, "tcp://%s:%d", host.c_str(), port);

        std::cerr << "Open server at " << address << std::endl;
        
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
    }


    void Server::sendMessage( const std::string& message )
    {
        // create a new NNG message
        nng_msg* msg;
        int rv = nng_msg_alloc(&msg, 0);
        if (rv != 0)
        {
            // handle error
            throw std::runtime_error("Could not allocate message.");
        }


        // rv = nng_msg_append(msg, "mrv2", strlen("mrv2") + 1);
        // if (rv != 0)
        // {
        //     // handle error
        //     nng_msg_free(msg);
        //     throw std::runtime_error("Could not set message header.");
        // }
    
        // set the message body to the binary data
        rv = nng_msg_append(
            msg, message.c_str(),
            message.length()); // exclude null terminator
        if (rv != 0)
        {
            // handle error
            nng_msg_free(msg);
            throw std::runtime_error("Could not set message body.");
        }
        ;
        // send the message
        rv = nng_sendmsg(sock, msg, 0);
        if (rv != 0)
        {
            // handle error
            nng_msg_free(msg);
            throw std::runtime_error("Could not send message.");
        }

        std::cerr << "Sent message " << message << std::endl;
        
        // free the message
        nng_msg_free(msg);

    }

} // namespace mrv
