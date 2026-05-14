// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvWebRTCClient.h"

namespace mrv
{
    WebRTCClient::WebRTCClient(const std::string& roomId,
                               const std::string& playerId)
    {
        bool enableDebugLogs = false;
        if (enableDebugLogs) {
            rtc::InitLogger(rtc::LogLevel::Debug);
        }
        

        rtc::Configuration config;
        std::string stunServer = "stun:stun.l.google.com:19302";
        std::cout << "STUN server is " << stunServer << std::endl;
        config.iceServers.emplace_back(stunServer);
        config.disableAutoNegotiation = true;

        webrtcManager.setConfiguration(config);

        // WebRTC → Signaling
        webrtcManager.onSignalMessage = [&](const SignalingMessage& msg) {
            signalingClient.send(msg);
        };

        // Signaling → WebRTC
        signalingClient.onInitPeer = [&](const std::string& peerId, bool isOfferer) {
            webrtcManager.createPeer(peerId, isOfferer);
        };

        signalingClient.onOffer = [&](const std::string& peerId, const std::string& sdp) {
            webrtcManager.handleOffer(peerId, sdp);
        };
    
        signalingClient.onAnswer = [&](const std::string& peerId, const std::string& sdp) {
            webrtcManager.handleAnswer(peerId, sdp);
        };

        signalingClient.onRemoteCandidate = [&](const std::string& peerId,
                                                const rtc::Candidate& c) {
            webrtcManager.addRemoteCandidate(peerId, c);  // owns pendingCandidates
        };

        signalingClient.onPeerDisconnected = [&](const std::string& peerId) {
            webrtcManager.erase(peerId);
        };
                                
        signalingClient.connect(roomId, playerId);
    }
    
    WebRTCClient::~WebRTCClient()
    {
    }

    void WebRTCClient::pushMessage(const Message&)
    {
    }

    void WebRTCClient::sendMessages()
    {
    }
    
    void WebRTCClient::receiveMessages()
    {
    }

} // namespace mrv
