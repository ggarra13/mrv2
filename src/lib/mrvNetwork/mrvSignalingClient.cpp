
#include "mrvNetwork/mrvSignalingClient.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"

#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

namespace
{

    std::string generateRandomLetters(int length = 6) {
        // Define the character set to choose from
        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
        // Setup the random number generator
        std::random_device rd;  // Obtain a random seed from the hardware
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    
        // Define the range (0 to the last index of the alphabet string)
        std::uniform_int_distribution<size_t> distrib(0, alphabet.size() - 1);

        std::string result = "";
        
        // Generate the random letters
        for (int i = 0; i < length; ++i) {
            result += alphabet[distrib(gen)];
        }
    
        return result;
    }

}

namespace mrv
{

    void SignalingClient::connect(const std::string& room,
                                  const std::string& player)
    {
        roomId = room;
    
        if (roomId.empty())
            roomId = generateRandomLetters();
    
        if (player.empty())
            playerId = "player" + generateRandomLetters(4);
        else
            playerId = player;
    
        const std::string url = "wss://srv1037957.hstgr.cloud/sync/" +
                                roomId + "/" + playerId;
        std::cout << "The room   ID is: " << roomId << std::endl;
        std::cout << "The player ID is: " << playerId << std::endl;

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
            std::cout << "WebSocket connected, signaling ready" << std::endl;
        });

        websocket->onClosed([]() {
            std::cout << "WebSocket closed" << std::endl;
        });

        websocket->onError([](const std::string &error) {
            std::cout << "WebSocket failed: " << error << std::endl;
        });
    
        websocket->onMessage([&](std::variant<rtc::binary, std::string> data) {
            if (!std::holds_alternative<std::string>(data))
                return;

            nlohmann::json message = nlohmann::json::parse(std::get<std::string>(data));
            handleMessage(message);
        });
    
        websocket->open(url);

        std::cout << "Waiting for signaling to be connected..." << std::endl;
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
            std::cout << "[" << playerId << "] Received offer from "
                      << sender_id << ".  Generating ANSWER." << std::endl;
            if (onOffer) onOffer(sender_id, message["sdp"].get<std::string>());
        }
        else if (type == "answer") {
            std::cout << "[" << playerId << "] Received answer from " << sender_id
                      << ". WebRTC Tunnel OPEN!" << std::endl;
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
