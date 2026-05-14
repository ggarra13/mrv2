
#include "mrvNetwork/mrvSignalingClient.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvUtil.h"

#include <tlCore/StringFormat.h>

#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

namespace
{
    const char* kModule = "w3tc";
}

namespace mrv
{

    void SignalingClient::connect(const std::string& roomId,
                                  const std::string& player)
    {
        if (player.empty())
            playerId = "player" + generateRandomLetters(4);
        else
            playerId = player;
    
        const std::string url = "wss://srv1037957.hstgr.cloud/sync/" +
                                roomId + "/" + playerId;
        
        std::string msg = string::Format(_("The room ID is: {0}")).arg(roomId);
        LOG_STATUS(msg);
        
        msg = string::Format(_("The player ID is: {0}")).arg(playerId);
        LOG_STATUS(msg);
        
        rtc::WebSocketConfiguration config;
        
        std::string caLocation = mrv::rootpath() + "/certs/cacert.pem";
        if (!file::isReadable(caLocation))
        {
#ifdef __linux___
            caLocation = "/etc/ssl/certs/ca-certificates.crt";
#elif defined(__APPLE__)
            caLocation = "/usr/local/etc/openssl@3/cert.pem";
#else
            caLocation = "";
#endif
        }

        if (!caLocation.empty())
            config.caCertificatePemFile = caLocation;
        
        websocket = std::make_shared<rtc::WebSocket>(config);
    
        websocket->onOpen([]() {
            LOG_STATUS("WebSocket connected, signaling ready");
        });

        websocket->onClosed([]() {
            LOG_STATUS("WebSocket closed.");
        });

        websocket->onError([](const std::string &error) {
            LOG_ERROR("WebSocket failed: " << error);
        });
    
        websocket->onMessage([&](std::variant<rtc::binary, std::string> data) {
            if (!std::holds_alternative<std::string>(data))
                return;

            nlohmann::json message = nlohmann::json::parse(std::get<std::string>(data));
            handleMessage(message);
        });
    
        websocket->open(url);

        while (!websocket->isOpen()) {
            if (websocket->isClosed())
                return;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void SignalingClient::send(const SignalingMessage& msg)
    {
        nlohmann::json j;
    
        j["id"] = msg.peerId;
        j["type"] = msg.type;

        if (!msg.sdp.empty())
            j["sdp"] = msg.sdp;

        if (!msg.candidate.empty())
            j["candidate"] = msg.candidate;

        if (!msg.mid.empty())
            j["mid"] = msg.mid;
    
        websocket->send(j.dump());
    }

    void SignalingClient::handleMessage(nlohmann::json message)
    {
        std::string sender_id = message.value("id", "unknown");
        std::string type      = message.value("type", "");

        if (type == "init_mesh") {
            auto peers = message["peers"].get<std::vector<std::string>>();
            for (auto& peer_id : peers)
                if (onInitPeer) onInitPeer(peer_id, /*isOfferer=*/true);
        }
        else if (type == "new_peer") {
            // They'll send us an offer; nothing to do except log
            if (onInitPeer) onInitPeer(sender_id, /*isOfferer=*/false);
        }
        else if (type == "offer") {
            LOG_STATUS("[" << playerId << "] Received offer from "
                       << sender_id << ".  Generating ANSWER.");
            if (onOffer) onOffer(sender_id, message["sdp"].get<std::string>());
        }
        else if (type == "answer") {
            LOG_STATUS("[" << playerId << "] Received answer from "
                       << sender_id
                       << ". WebRTC Tunnel OPEN!");
            if (onAnswer) onAnswer(sender_id, message["sdp"].get<std::string>());
        }
        else if (type == "candidate") {
            rtc::Candidate c(message["candidate"].get<std::string>(),
                             message["mid"].get<std::string>());
            if (onRemoteCandidate) onRemoteCandidate(sender_id, c);
        }
        else if (type == "peer_disconnected") {
            if (onPeerDisconnected) onPeerDisconnected(sender_id);
        }
    }

}
