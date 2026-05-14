
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
        bool dataChannelOpen = false;
        bool sentRemote = false;

        std::string createOffer()
            {
                return _peerConnection->createOffer();
            }
        
        void setRemoteDescription(const rtc::Description& d)
            {
                _peerConnection->setRemoteDescription(d);
                sentRemote = true;
            }
    
    private:
        bool _sentRemote = false;
        std::shared_ptr<rtc::PeerConnection> _peerConnection;
    };

}
