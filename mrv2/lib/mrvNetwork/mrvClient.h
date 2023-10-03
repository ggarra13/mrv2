// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class Client : public TCP
    {
    public:
        Client(const std::string& ip = "localhost", const uint16_t port = 5800);
        virtual ~Client();

        void sendMessages() override;
        void receiveMessages() override;

        std::string host() const { return m_host; }

    private:
        std::string m_host;
    };
} // namespace mrv
