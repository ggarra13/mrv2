/**
 * libdatachannel client example
 * Copyright (c) 2019-2020 Paul-Louis Ageneau
 * Copyright (c) 2019 Murat Dogan
 * Copyright (c) 2020 Will Munn
 * Copyright (c) 2020 Nico Chatzi
 * Copyright (c) 2020 Lara Mackey
 * Copyright (c) 2020 Erik Cota-Robles
 * Copyright (c) 2020 Filip Klembara (in2core)
 * Copyright (c) 2026 Gonzalo Garramuño
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "WebRTCManager.h"
#include "SignalingClient.h"

#include <iostream>
#include <string>

int main(int argc, char **argv) try {

    if (argc != 3)
    {
        std::cout << argv[0] << " <room> <player>" << std::endl;
        exit(1);
    }
    
    

    std::string roomId = "roomA";
    std::string playerId = "player";

    roomId = argv[1];
    playerId = argv[2];


    bool enableDebugLogs = false;
    if (enableDebugLogs) {
        rtc::InitLogger(rtc::LogLevel::Debug);
    }

    rtc::Configuration config;
    std::string stunServer = "stun:stun.l.google.com:19302";
    std::cout << "STUN server is " << stunServer << std::endl;
    config.iceServers.emplace_back(stunServer);
    config.disableAutoNegotiation = true;

    WebRTCManager webrtcManager(config);    
    SignalingClient signalingClient;

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

    while (true) {
        std::string id;
        std::cout << "q + Enter to exit" << std::endl;
        std::cin >> id;
        std::cin.ignore();
        std::cout << "exiting" << std::endl;
        break;
    }

    std::cout << "Cleaning up..." << std::endl;
    return 0;

} catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
}

