
#pragma once

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class Server : public TCP
    {
    public:
        Server(const std::string& host = "localhost", unsigned port = 5800);

        void sendMessage( const std::string& msg ) override;
    };
} // namespace mrv
