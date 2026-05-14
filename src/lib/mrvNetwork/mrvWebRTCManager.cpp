#include "mrvNetwork/mrvWebRTCManager.h"

#include "mrvFl/mrvIO.h"

namespace
{
    template <class T>
    std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }

    const char* kModule = "w3tc";
}

namespace mrv
{

    WebRTCManager::WebRTCManager()
    {
    }
    
    void WebRTCManager::setConfiguration(const rtc::Configuration& value)
    {
        config = value;
    }
    
    void WebRTCManager::publish(const Message& msg)
    {
        std::vector< uint8_t > bson = nlohmann::json::to_bson(msg);
        std::size_t messageLength = bson.size();
        if (messageLength == 0)
            return;
                
        for (auto& [_, client] : clients)
        {
            if (!client->dataChannelOpen)
                continue;
            
            client->dataChannel->send(
                reinterpret_cast<const std::byte*>(bson.data()),
                messageLength);
        }
    }
    
    std::shared_ptr<WebRTCConnection>
    WebRTCManager::createPeer(const std::string id,
                              const bool isOfferer)
    {
        using namespace rtc;
    
        std::shared_ptr<WebRTCConnection> client;
        auto pc = std::make_shared<PeerConnection>(config);
        client = std::make_shared<WebRTCConnection>(pc);
        {
            std::lock_guard<std::mutex> lock(mtx);
            clients[id] = client;
        }
        pc->onStateChange([this, id, pc](PeerConnection::State state) {
            if (state == PeerConnection::State::Failed)
            {
                LOG_ERROR("[" << id << "] State: " << state);
            }
            else
            {
                LOG_STATUS("[" << id << "] State: " << state);
            }
                
            if (state == PeerConnection::State::Disconnected ||
                state == PeerConnection::State::Failed ||
                state == PeerConnection::State::Closed) {
                // remove disconnected client
                erase(id);
            }
            else
            {
                drainPendingCandidates(id);
            }
        });

        pc->onLocalCandidate([this, id](Candidate candidate) {
            SignalingMessage msg;
            msg.peerId = id;
            msg.type = "candidate";
            msg.candidate = std::string(candidate);
            msg.mid = candidate.mid();

            if (onSignalMessage)
            {
                onSignalMessage(msg);
            }
        });

        pc->onGatheringStateChange(
            [this, wpc = make_weak_ptr(pc), id](PeerConnection::GatheringState state) {
                LOG_STATUS("Gathering State: " << state);
                if (state == PeerConnection::GatheringState::Complete) {
                    if(auto pc = wpc.lock()) {
                        auto description = pc->localDescription();
                
                        SignalingMessage msg;
                        msg.peerId = id;
                        msg.type = description->typeString();
                        msg.sdp = std::string(description.value());
                
                        if (onSignalMessage)
                        {
                            onSignalMessage(msg);
                        }
                    }
                }
            });

        // Handle incoming DataChannel
        pc->onDataChannel([this, id, client](std::shared_ptr<DataChannel> dc) {

            client->dataChannel = dc;

            dc->onOpen([id, client]() {
                client->dataChannelOpen = true;
            });

            dc->onMessage([this, id](const rtc::binary data) {
                if (onBinaryMessage)
                {
                    onBinaryMessage(data);
                }
            }, {});

            dc->onClosed([id, client]() {
                client->dataChannelOpen = false;
            });
        });
    
        if (isOfferer)
        {
            auto dc = pc->createDataChannel("mrv2_sync");

            dc->onOpen([id, client]() {
                client->dataChannelOpen = true;
            });
            
            dc->onMessage([this, id](const rtc::binary data) {
                if (onBinaryMessage)
                {
                    onBinaryMessage(data);
                }
            }, {});

            dc->onClosed([id, client]() {
                client->dataChannelOpen = false;
            });
            client->dataChannel = dc;
        
            pc->setLocalDescription();
        }
    
        return client;
    }



    void WebRTCManager::handleOffer(const std::string& peerId, const std::string& sdp)
    {
        auto client = createPeer(peerId, /*isOfferer*/ false);
    
        auto description = rtc::Description(sdp, "offer");
        client->setRemoteDescription(description);
    
        auto pc = client->peerConnection;
        pc->setLocalDescription();

        drainPendingCandidates(peerId);
    }

    void WebRTCManager::handleAnswer(const std::string& peerId, const std::string& sdp)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (auto jt = clients.find(peerId); jt != clients.end()) {
            auto client = jt->second;
            auto description = rtc::Description(sdp, "answer");
            client->setRemoteDescription(description);
        
            drainPendingCandidates(peerId);
        }
    }

    void WebRTCManager::addRemoteCandidate(const std::string& peerId, const rtc::Candidate& c)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto jt = clients.find(peerId);
        if (jt != clients.end() && jt->second->sentRemote)
        {
            jt->second->peerConnection->addRemoteCandidate(c);
        }
        else
        {
            // PC not created or sendRemoteDescription not sent yet — buffer it
            pendingCandidates[peerId].push_back(c);
        }
    }

    void WebRTCManager::erase(const std::string& peerId)
    {
        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(peerId);
        pendingCandidates.erase(peerId);
    }

    void WebRTCManager::drainPendingCandidates(const std::string& peerId)
    {
        // Drain any candidates that arrived before we were ready.
        // DO NOT ADD std::lock_guard lock(mtx).
        auto i = clients.find(peerId);
        if (i == clients.end())
            return;

        auto client = i->second;
        auto pc = client->peerConnection;
    
        if (auto it = pendingCandidates.find(peerId);
            it != pendingCandidates.end()) {
            for (auto& c : it->second)
                pc->addRemoteCandidate(c);
            pendingCandidates.erase(it);
        }
    }

}
