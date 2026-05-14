
#include "mrvNetwork/mrvSignalingMessage.h"

#include <rtc/rtc.hpp>

#include <nlohmann/json.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace mrv
{

    class SignalingClient
    {
    public:
        void connect(const std::string& roomId,
                     const std::string& playerId = "");

        void send(const SignalingMessage& msg);
    
        // — Callbacks wired by main.cpp —
        std::function<void(const std::string& peerId, bool isOfferer)>
        onInitPeer;      // init_mesh or new_peer → create a peer

        std::function<void(const std::string& peerId,
                           const std::string& sdp)>
        onOffer;         // remote sent us an offer

        std::function<void(const std::string& peerId,
                           const std::string& sdp)>
        onAnswer;        // remote sent us an answer

        std::function<void(const std::string& peerId,
                           const rtc::Candidate&)>
        onRemoteCandidate; // ICE candidate arrived

        std::function<void(const std::string& peerId)>
        onPeerDisconnected;

    protected:
        void handleMessage(nlohmann::json message);
    
        std::shared_ptr<rtc::WebSocket> websocket;
        std::string roomId;
        std::string playerId;
    };

}
