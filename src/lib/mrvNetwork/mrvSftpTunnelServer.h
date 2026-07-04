// mrvSftpTunnelServer.h
#pragma once
#include "mrvSftpTunnelPump.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <vector>
#include <mutex>
#include <algorithm>

namespace mrv
{
    // Registers itself on WebRTCManager::onExtraDataChannel. Whenever
    // any peer opens a "sftp-tunnel" channel, connects to this
    // machine's own sshd and pumps bytes between them.
    class SftpTunnelServer
    {
    public:
        explicit SftpTunnelServer(WebRTCManager& manager,
                                   Poco::UInt16 sshdPort = 22)
            : sshdPort_(sshdPort)
        {
            manager.onExtraDataChannel =
                [this](const std::string& peerId,
                       std::shared_ptr<rtc::DataChannel> dc)
                {
                    if (dc->label() == "sftp-tunnel")
                        handleIncoming(peerId, dc);
                };
        }

    private:
        void handleIncoming(const std::string& peerId,
                            std::shared_ptr<rtc::DataChannel> channel);

        Poco::UInt16 sshdPort_;
        std::mutex pumpsMutex_;
        std::vector<std::shared_ptr<TcpDataChannelPump>> pumps_;
    };
} // namespace mrv
