#pragma once

#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvSignalingMessage.h"
#include "mrvNetwork/mrvWebRTCConnection.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace mrv
{

    class WebRTCManager
    {
    public:
        WebRTCManager();

        void setConfiguration(const rtc::Configuration&);

        std::shared_ptr<WebRTCConnection> createPeer(const std::string peerId,
                                                     bool isOfferer);


        std::shared_ptr<WebRTCConnection> getClient(const std::string& peerId);

        void handleOffer(const std::string& peerId, const std::string& sdp);

        void handleAnswer(const std::string& peerId, const std::string& sdp);

        void addRemoteCandidate(const std::string& peerId, const rtc::Candidate& c);

        void publish(const Message&);

        std::function<void(const std::string& peerId, const rtc::binary&)> onBinaryMessage;
        std::function<void(const std::string& peerId, const std::string&)> onStringMessage;

        // Fired for any incoming DataChannel whose label isn't "mrv2_sync".
        // Only one consumer at a time (last one registered wins) — fine
        // while SFTP tunneling is the only such consumer, but worth
        // revisiting if a second non-sync channel type gets added later.
        std::function<void(const std::string& peerId,
                        std::shared_ptr<rtc::DataChannel>)> onExtraDataChannel;

        std::function<void(const SignalingMessage&)>
        onSignalMessage;

        std::function<void(const std::string& peerId)> onPeerDisconnected;

        void erase(const std::string& peerId);

    protected:
        void drainPendingCandidates(const std::string& peerId);

        rtc::Configuration config;
        std::unordered_map<std::string, std::shared_ptr<WebRTCConnection> > clients;
        std::unordered_map<std::string, std::vector<rtc::Candidate> > pendingCandidates;

        std::mutex mtx;
    };

}
