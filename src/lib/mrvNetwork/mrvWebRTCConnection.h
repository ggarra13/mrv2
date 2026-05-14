
#pragma once

#include "rtc/rtc.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace mrv
{

    struct WebRTCConnection {
        const std::shared_ptr<rtc::PeerConnection> & peerConnection = _peerConnection;
        WebRTCConnection(std::shared_ptr<rtc::PeerConnection> pc) {
            _peerConnection = pc;
        }

        std::shared_ptr<rtc::DataChannel> dataChannel;

        std::string createOffer()
            {
                return _peerConnection->createOffer();
            }

        bool sentRemote() const
            {
                return _sentRemote;
            }
    
        void setRemoteDescription(const rtc::Description& d)
            {
                _peerConnection->setRemoteDescription(d);
                _sentRemote = true;
            }
    
    private:
        std::atomic<bool> _sentRemote = false;
        std::shared_ptr<rtc::PeerConnection> _peerConnection;
    };

}
