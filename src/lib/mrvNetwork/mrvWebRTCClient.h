// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvSignalingClient.h"
#include "mrvNetwork/mrvWebRTCManager.h"

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class WebRTCClient : public TCP
    {
    public:
        WebRTCClient(const std::string& room, const std::string& player = "");
        virtual ~WebRTCClient();

        void pushMessage(const Message&) override;
        void sendMessages() override;
        void receiveMessages() override;

    protected:
        SignalingClient signalingClient;
        WebRTCManager webrtcManager;
    };
} // namespace mrv
