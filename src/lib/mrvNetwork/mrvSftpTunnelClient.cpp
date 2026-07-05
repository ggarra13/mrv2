
#include "mrvNetwork/mrvSftpTunnelClient.h"

#include "mrvFl/mrvIO.h"

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "sftp";
}

namespace mrv
{
    void SftpTunnelClient::acceptLoop()
    {
        while (running_)
        {
            Poco::Net::StreamSocket client;
            try
            {
                client = serverSocket_.acceptConnection();
            }
            catch (const Poco::Exception& e)
            {
                break; // listener closed (stop()) or real error
            }

            auto peer = manager_.getClient(peerId_);
            if (!peer || !peer->peerConnection)
            {
                std::string msg =
                    tl::string::Format("SFTP tunnel: peer {0} is not "
                                       "connected.").arg(peerId_);
                LOG_ERROR(msg);
                client.close();
                running_ = false;
                continue;
            }

            rtc::DataChannelInit init;
            init.reliability.unordered = false; // reliable + ordered

            auto channel =
                peer->peerConnection->createDataChannel("sftp-tunnel", init);

            channel->onOpen(
                [this, client, channel]() mutable
                    {
                        auto pump = TcpDataChannelPump::create(client, channel);
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
                    });

            channel->onError(
                [client](std::string err) mutable
                    {
                        LOG_ERROR("SFTP tunnel channel error: " << err);
                        client.close();
                    });
        }
    }

} // namespace mrv
