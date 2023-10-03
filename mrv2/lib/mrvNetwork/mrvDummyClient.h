// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class DummyClient : public TCP
    {
    public:
        DummyClient(){};
        virtual ~DummyClient(){};

        void pushMessage(const Message&) override{};
        void sendMessages() override{};
        void receiveMessages() override{};
    };
} // namespace mrv
