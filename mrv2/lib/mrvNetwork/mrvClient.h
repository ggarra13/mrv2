
#pragma once

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class Client : public TCP
    {
    public:
        Client(const std::string& ip = "localhost", unsigned port = 5800);

        void receiveMessage() override;
    };
} // namespace mrv
