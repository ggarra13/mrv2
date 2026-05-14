#include "mrvNetwork/mrvWebRTCManager.h"

namespace
{
    template <class T>
    std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }
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
            std::cout << id << " State: " << state << std::endl;
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
                std::cout << "Gathering State: " << state << std::endl;
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

        // IMPORTANT: Handle incoming DataChannel (especially on answerer side)
        pc->onDataChannel([id, client](std::shared_ptr<DataChannel> dc) {
            std::cout << "[" << id << "] Received remote DataChannel: " 
                      << dc->label() << std::endl;

            client->dataChannel = dc;

            dc->onOpen([id, client]() {
                std::cout << "[" << id << "] DataChannel onOpen" << std::endl;
            });

            // dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](
            //                   const std::string& msg) {
            //     std::cout << "Message from " << id << " received 1: " << msg << std::endl;
            //     // Echo back
            //     if (auto dc = wdc.lock()) {
            //         dc->send("Pong");
            //     }
            // });

            dc->onClosed([id]() {
                std::cout << "[" << id << "] DataChannel closed" << std::endl;
            });
        });
    
        if (isOfferer)
        {
            auto dc = pc->createDataChannel("mrv2_sync");
            dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
                if (auto dc = wdc.lock()) {
                    dc->send("Ping");
                }
            });

            // dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
            //     std::cout << "Message from " << id << " received: " << msg
            //               << std::endl;
            //     if (auto dc = wdc.lock()) {
            //         dc->send("Ping");
            //     }
            // });
    
            dc->onClosed([id, wdc = make_weak_ptr(dc)]() {
                std::cout << "onClosed" << std::endl;
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
        if (jt != clients.end() && jt->second->sentRemote())
        {
            jt->second->peerConnection->addRemoteCandidate(c);
        }
        else
        {
            // PC not created or sendRemoteDescription sent yet — buffer it
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
        // DO NOT ADD lock(mtx).
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
