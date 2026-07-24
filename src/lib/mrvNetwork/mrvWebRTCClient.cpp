// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvApp.h"

#include "mrvNetwork/mrvWebRTCClient.h"
#include "mrvNetwork/mrvFileTransferServer.h"

#include "mrvFl/mrvIO.h"

#include "mrvOS/mrvOS.h"

#include <tlCore/StringFormat.h>

#include "mrViewer.h"

namespace
{
    const char* kModule = "w3tc";
}

namespace mrv
{
    WebRTCClient::WebRTCClient(const std::string& studio,
                               const std::string& roomId,
                               const std::string& playerId)
    {
        std::string debug = os::sgetenv("MRV2_WEBRTC_DEBUG");
        if (debug == "1" || debug == "ON") {
            rtc::InitLogger(rtc::LogLevel::Debug);
        }


        std::string stunServer = os::sgetenv("MRV2_STUN_SERVER");
        if (stunServer.empty())
            stunServer = App::ui->uiPrefs->uiPrefsWebRTCStunServer->value();

        std::string msg = string::Format(_("STUN server is {0}")).
                          arg(stunServer);
        LOG_STATUS(msg);

        rtc::Configuration config;
        if (!stunServer.empty())
            config.iceServers.emplace_back(stunServer);
        config.disableAutoNegotiation = true;

        webrtcManager.setConfiguration(config);

        // Every mesh participant must be ready to serve a file to any other
        // peer, regardless of whether this machine ever needs to fetch one
        // itself - construct unconditionally, not on demand.
        fileServer = FileTransferServer::create(webrtcManager);


        // WebRTC → WebRTCClient (this class)
        webrtcManager.onBinaryMessage = [&](const std::string& peerId,
                                            const rtc::binary& data)
            {
                std::lock_guard lk(m_receiveMutex);
                Message message = nlohmann::json::from_bson(data);
                message[kLocalPeerIdKey] = peerId;
                m_receive.push_back(message);
            };

        webrtcManager.onStringMessage = [&](const std::string& peerId,
                                            const std::string& msg)
            {
                std::lock_guard lk(m_receiveMutex);
                Message message = nlohmann::json::parse(msg);
                m_receive.push_back(message);
            };

        webrtcManager.onPeerDisconnected = [&](const std::string& peerId)
            {
                std::lock_guard lk(m_receiveMutex);
                Message message;
                message["command"] = "Peer Disconnected";
                message[kLocalPeerIdKey] = peerId;
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

        signalingClient.connect(studio, roomId, playerId);
    }

    WebRTCClient::~WebRTCClient()
    {
    }

    void WebRTCClient::pushMessage(const Message& message)
    {
        // Only push messages if we are not locked.
        if (m_lock)
            return;
        std::lock_guard lk(m_sendMutex);
        webrtcManager.publish(message);
    }

    void WebRTCClient::pushToPeer(const std::string& peerId,
                                  const Message& message)
    {
        // Only push messages if we are not locked.
        if (m_lock)
            return;

        if (peerId.empty())
            return pushMessage(message);

        std::lock_guard lk(m_sendMutex);
        webrtcManager.pushMessageToPeer(peerId, message);
    }

    void WebRTCClient::sendMessages()
    {
        // Not used.  We use publish directly.
    }

    void WebRTCClient::receiveMessages()
    {
        // This is handled by a WebRTCManager's dataChannel's callback
    }

} // namespace mrv
