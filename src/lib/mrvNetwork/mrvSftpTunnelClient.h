// mrvSftpTunnelClient.h
#pragma once

#include "mrvFl/mrvIO.h"

#include "mrvSftpTunnelPump.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>


namespace mrv
{
    // Opens a local TCP listener on 127.0.0.1:<localPort>. Point an
    // SFTP client at that address; each accepted local connection
    // opens a fresh "sftp-tunnel" DataChannel to `peerId`, looked up
    // live from WebRTCManager (not cached), so a reconnect with a new
    // PeerConnection under the same id is picked up automatically.
    class SftpTunnelClient
    {
    public:
        SftpTunnelClient(WebRTCManager& manager, std::string peerId,
                          Poco::UInt16 localPort = 2222)
            : manager_(manager)
            , peerId_(std::move(peerId))
            , serverSocket_(Poco::Net::SocketAddress("127.0.0.1", localPort))
        {
        }

        Poco::UInt16 localPort() const {
            return serverSocket_.address().port();
        }

        void start()
        {
            acceptThread_ = std::thread([this]() { acceptLoop(); });
        }

        void stop()
        {
            running_ = false;
            try { serverSocket_.close(); } catch (const Poco::Exception&) {}
            if (acceptThread_.joinable())
                acceptThread_.join();

            std::lock_guard<std::mutex> lk(pumpsMutex_);
            for (auto& pump : pumps_)
                pump->stop();
            pumps_.clear();
        }

        ~SftpTunnelClient() { stop(); }

    private:
        void acceptLoop();

        WebRTCManager& manager_;
        std::string peerId_;
        Poco::Net::ServerSocket serverSocket_;
        std::thread acceptThread_;
        std::atomic<bool> running_{true};

        std::mutex pumpsMutex_;
        std::vector<std::shared_ptr<TcpDataChannelPump>> pumps_;
    };
} // namespace mrv
