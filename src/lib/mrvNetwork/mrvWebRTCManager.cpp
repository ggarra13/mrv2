#include "mrvNetwork/mrvWebRTCManager.h"

#include "mrvFl/mrvIO.h"

#include <rtc/global.hpp>

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
        rtc::SctpSettings sctpSettings;
        sctpSettings.recvBufferSize = 4 * 1024 * 1024;      // default is much smaller
        sctpSettings.sendBufferSize = 4 * 1024 * 1024;
        sctpSettings.maxChunksOnQueue = 16384;
        sctpSettings.initialCongestionWindow = 10;          // in MTUs, default is very small (often ~3)
        rtc::SetSctpSettings(sctpSettings);

        eraseThread_ = std::thread(&WebRTCManager::eraseWorker, this);
    }

    WebRTCManager::~WebRTCManager()
    {
        {
            std::lock_guard<std::mutex> lock(eraseQueueMutex_);
            stopping_ = true;
        }
        eraseQueueCv_.notify_all();

        // guaranteed no thread touches `this` after this point
        if (eraseThread_.joinable())
            eraseThread_.join();
    }

    void WebRTCManager::requestErase(const std::string& peerId)
    {
        std::lock_guard<std::mutex> lock(eraseQueueMutex_);
        eraseQueue_.push_back(peerId);
        eraseQueueCv_.notify_one();
    }

    void WebRTCManager::eraseWorker()
    {
        while (true)
        {
            std::string peerId;
            {
                std::unique_lock<std::mutex> lock(eraseQueueMutex_);
                eraseQueueCv_.wait(lock, [this] {
                    return stopping_ || !eraseQueue_.empty();
                });
                if (stopping_ && eraseQueue_.empty())
                    return;
                peerId = eraseQueue_.front();
                eraseQueue_.pop_front();
            }
            erase(peerId);   // existing body, unchanged
        }
    }

    void WebRTCManager::setConfiguration(const rtc::Configuration& value)
    {
        config = value;
    }

    void WebRTCManager::pushMessageToPeer(const Message& msg,
                                          const std::string& peerId)
    {
        std::vector< uint8_t > bson = nlohmann::json::to_bson(msg);
        std::size_t messageLength = bson.size();
        if (messageLength == 0)
            return;

        auto i = clients.find(peerId);
        if (i != clients.end())
        {
            auto client = i->second;
            if (!client->dataChannelOpen)
                return;

            client->dataChannel->send(
                reinterpret_cast<const std::byte*>(bson.data()),
                messageLength);
        }
    }

    void WebRTCManager::publish(const Message& msg)
    {
        std::vector< uint8_t > bson = nlohmann::json::to_bson(msg);
        std::size_t messageLength = bson.size();
        if (messageLength == 0)
            return;

        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

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

        auto pc = std::make_shared<PeerConnection>(config);
        auto client = std::make_shared<WebRTCConnection>(pc);
        {
            std::lock_guard<std::mutex> lock(mtx);
            clients[id] = client;
        }

        std::weak_ptr<WebRTCConnection> wclient = client;

        pc->onStateChange([this, wclient, id](PeerConnection::State state) {

            if (state == PeerConnection::State::Failed)
            {
                LOG_STATUS("[" << id << "] State: " << state);
            }
            else
            {
                LOG_STATUS("[" << id << "] State: " << state);
            }

            if (state == PeerConnection::State::Disconnected ||
                state == PeerConnection::State::Failed ||
                state == PeerConnection::State::Closed) {
                requestErase(id);
            }
            else
            {
                auto client = wclient.lock();
                if (!client)
                    return;

                std::lock_guard<std::mutex> lock(mtx);
                drainPendingCandidates(id);

                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                rtc::Candidate local, remote;
                auto pair = client->peerConnection->getSelectedCandidatePair(&local, &remote);
                if (pair)
                {
                    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                    client->isRelayedConnection =
                        (local.type() == rtc::Candidate::Type::Relayed ||
                         remote.type() == rtc::Candidate::Type::Relayed);
                    const std::string connType = client->isRelayedConnection ? "relayed" : "direct";
                    LOG_STATUS("[" << id << "] ICE connection type: "
                               << connType);

                }
            }
        });

        pc->onLocalCandidate([this, id](Candidate candidate) {
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            SignalingMessage msg;
            msg.peerId = id;
            msg.type = "candidate";
            msg.candidate = std::string(candidate);
            msg.mid = candidate.mid();

            if (onSignalMessage)
            {
                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                onSignalMessage(msg);
            }
        });

        pc->onGatheringStateChange(
            [this, wpc = make_weak_ptr(pc), id](PeerConnection::GatheringState state) {
                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                LOG_STATUS("Gathering State: " << state);
                if (state == PeerConnection::GatheringState::Complete) {
                    if(auto pc = wpc.lock()) {
                        auto description = pc->localDescription();

                        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                        SignalingMessage msg;
                        msg.peerId = id;
                        msg.type = description->typeString();
                        msg.sdp = std::string(description.value());

                        if (onSignalMessage)
                        {
                            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                            onSignalMessage(msg);
                        }
                    }
                }
            });

        // Handle incoming DataChannel
        pc->onDataChannel([this, id, wclient](std::shared_ptr<DataChannel> dc) {

            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (dc->label() != "mrv2_sync")
            {
                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                if (onExtraDataChannel)
                    onExtraDataChannel(id, dc);
                return;
            }

            auto client = wclient.lock();
            if (!client) return;

            client->dataChannel = dc;

            dc->onOpen([id, wclient]() {
                auto client = wclient.lock();
                if (!client) return;

                client->dataChannelOpen = true;

                nlohmann::json message;
                message["command"] = "sync";

                const std::string s = message.dump();

                client->dataChannel->send(s);
            });

            dc->onMessage(
                [this, id](const rtc::binary data) {
                    if (onBinaryMessage)
                        onBinaryMessage(id, data);
                },
                [this, id](const std::string& msg) {
                    if (onStringMessage)
                        onStringMessage(id, msg);
                });

            dc->onClosed([wclient]() {
                auto client = wclient.lock();
                if (!client) return;
                client->dataChannelOpen = false;
            });
        });

        if (isOfferer)
        {
            auto dc = pc->createDataChannel("mrv2_sync");

            dc->onOpen([wclient]() {
                auto client = wclient.lock();
                if (!client) return;

                client->dataChannelOpen = true;

                nlohmann::json message;
                message["command"] = "sync";

                std::string s = message.dump();

                client->dataChannel->send(s);
            });

            dc->onMessage(
                [this, id](const rtc::binary data) {
                    if (onBinaryMessage)
                    {
                        onBinaryMessage(id, data);
                    }
                },
                [this, id](const std::string& msg) {
                    if (onStringMessage)
                    {
                        onStringMessage(id, msg);
                    }
                });

            dc->onClosed([wclient]() {
                auto client = wclient.lock();
                if (!client) return;

                client->dataChannelOpen = false;
            });

            client->dataChannel = dc;
            pc->setLocalDescription();
        }

        return client;
    }


    std::shared_ptr<WebRTCConnection>
    WebRTCManager::getClient(const std::string& peerId)
    {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        std::lock_guard<std::mutex> lock(mtx);
        auto it = clients.find(peerId);
        if (it == clients.end())
            return nullptr;
        return it->second;
    }

    void WebRTCManager::handleOffer(const std::string& peerId, const std::string& sdp)
    {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        auto client = createPeer(peerId, /*isOfferer*/ false);

        auto description = rtc::Description(sdp, "offer");
        client->setRemoteDescription(description);

        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

        auto pc = client->peerConnection;
        pc->setLocalDescription();

        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        drainPendingCandidates(peerId);
    }

    void WebRTCManager::handleAnswer(const std::string& peerId, const std::string& sdp)
    {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        std::lock_guard<std::mutex> lock(mtx);
        if (auto jt = clients.find(peerId); jt != clients.end()) {
            auto client = jt->second;
            auto description = rtc::Description(sdp, "answer");
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            client->setRemoteDescription(description);

            drainPendingCandidates(peerId);
        }
    }

    void WebRTCManager::addRemoteCandidate(const std::string& peerId, const rtc::Candidate& c)
    {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        std::lock_guard<std::mutex> lock(mtx);
        auto jt = clients.find(peerId);
        if (jt != clients.end() && jt->second->sentRemote)
        {
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            jt->second->peerConnection->addRemoteCandidate(c);
        }
        else
        {
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            // PC not created or sendRemoteDescription not sent yet — buffer it
            pendingCandidates[peerId].push_back(c);
        }
    }

    void WebRTCManager::erase(const std::string& peerId)
    {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(peerId);
        pendingCandidates.erase(peerId);

        if (onPeerDisconnected)
        {
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            onPeerDisconnected(peerId);
        }
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

        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        if (auto it = pendingCandidates.find(peerId);
            it != pendingCandidates.end()) {
            for (auto& c : it->second)
                pc->addRemoteCandidate(c);
            pendingCandidates.erase(it);
        }
    }

}
