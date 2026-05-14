// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvWebRTCClient.h"

#include "mrvFl/mrvIO.h"

#include "mrvOS/mrvOS.h"

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "w3tc";
}

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
        std::string stunServer = os::sgetenv("MRV2_STUNSERVER");
        if (stunServer.empty())
            stunServer = "stun:stun.l.google.com:19302";

        std::string msg = string::Format(_("STUN server is {0}")).
                          arg(stunServer);
        LOG_STATUS(msg);
        
        config.iceServers.emplace_back(stunServer);
        config.disableAutoNegotiation = true;

        webrtcManager.setConfiguration(config);

        // WebRTC → WebRTCClient (this class)
        webrtcManager.onBinaryMessage = [&](const rtc::binary& data)
            {
                std::lock_guard lk(m_receiveMutex);
                Message message = nlohmann::json::from_bson(data);
                m_receive.push_back(message);
            };
        
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
            webrtcManager.addRemoteCandidate(peerId, c);
        };

        signalingClient.onPeerDisconnected = [&](const std::string& peerId) {
            webrtcManager.erase(peerId);
        };
                                
        signalingClient.connect(roomId, playerId);
    }
    
    WebRTCClient::~WebRTCClient()
    {
    }

    void WebRTCClient::pushMessage(const Message& message)
    {
        if (m_lock)
            return;
        std::lock_guard lk(m_sendMutex);
        webrtcManager.publish(message);
    }

    void WebRTCClient::sendMessages()
    {
    }
    
    void WebRTCClient::receiveMessages()
    {
    }

} // namespace mrv
