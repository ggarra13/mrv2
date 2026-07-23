// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvSignalingClient.h"
#include "mrvNetwork/mrvWebRTCManager.h"

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    class FileTransferServer;

    class WebRTCClient : public TCP
    {
    public:
        WebRTCClient(const std::string& studio,
                     const std::string& room, const std::string& player = "");
        virtual ~WebRTCClient();

        WebRTCManager& manager() { return webrtcManager; }

        void pushMessage(const Message&) override;
        void pushToPeer(const std::string& peerId,
                        const Message& message) override;
        void sendMessages() override;
        void receiveMessages() override;

        void handleBinaryMessage(rtc::binary data);

    protected:
        SignalingClient signalingClient;
        WebRTCManager webrtcManager;
        std::shared_ptr<FileTransferServer> fileServer;
    };
} // namespace mrv
