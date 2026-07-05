
#include "mrvNetwork/mrvSftpTunnelServer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "sftp";
}

namespace mrv
{

    void SftpTunnelServer::handleIncoming(
        const std::string& peerId,
        std::shared_ptr<rtc::DataChannel> channel)
    {
        Poco::Net::StreamSocket socket;
        try
        {
            socket.connect(
                Poco::Net::SocketAddress("127.0.0.1", sshdPort_));
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR("SFTP tunnel: could not reach local sshd on port "
                      << sshdPort_ << " for peer " << peerId
                      << ": " << e.displayText());
            channel->close();
            return;
        }

        auto pump = TcpDataChannelPump::create(socket, channel);
        std::weak_ptr<TcpDataChannelPump> weakPump = pump;
        pump->setOnFinished([this, weakPump]()
            {
                std::lock_guard<std::mutex> lk(pumpsMutex_);
                pumps_.erase(
                    std::remove_if(pumps_.begin(), pumps_.end(),
                                   [&](auto& p) { return p == weakPump.lock(); }),
                    pumps_.end());
            });
        pump->start();

        std::lock_guard<std::mutex> lk(pumpsMutex_);
        pumps_.push_back(pump);
    }
}
