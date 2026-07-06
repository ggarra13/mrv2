
#include "mrvNetwork/mrvMessage.h"

namespace mrv
{
    
    // Opens a "file-transfer" DataChannel to a specific peer and
    // reassembles the incoming chunks into a local file.
    class FileTransferClient
    {
    public:
        FileTransferClient(WebRTCManager& manager,
                           const std::string& peerId,
                           const std::string& remotePath,
                           const std::string& localPath,
                           std::function<void(uint64_t,uint64_t)> progressCb,
                           std::function<void(bool)> doneCb)
            : localPath_(localPath)
            , partPath_(localPath + ".part")
            , progressCb_(std::move(progressCb))
            , doneCb_(std::move(doneCb))
            {
                auto peer = manager.getClient(peerId);
                if (!peer)
                {
                    doneCb_(false);
                    return;
                }

                rtc::DataChannelInit init;
                init.reliability.unordered = false;

                auto dc = peer->peerConnection->createDataChannel(
                    "file-transfer", init);

                dc->onOpen([dc, remotePath]()
                    {
                        Message req;
                        req["path"] = remotePath;
                        dc->send(req.dump());
                    });

                dc->onMessage([this](rtc::message_variant msg)
                    {
                        if (std::holds_alternative<std::string>(msg))
                            handleText(std::get<std::string>(msg));
                        else
                            handleBinary(std::get<rtc::binary>(msg));
                    });

                dc->onClosed([this]() { finish(false); });
                dc->onError([this](std::string) { finish(false); });
            }

    private:
        void handleText(const std::string& text)
            {
                Message msg = nlohmann::json::parse(text);

                if (msg.contains("error"))
                {
                    LOG_ERROR(msg["error"].get<std::string>());
                    finish(false);
                }
                else if (msg.contains("size"))
                {
                    // Header — open the .part file for writing
                    remoteSize_ = msg["size"];
                    out_ = std::fopen(partPath_.c_str(), "wb");
                    if (!out_)
                    {
                        LOG_ERROR("Could not open " + partPath_);
                        finish(false);
                    }
                }
                else if (msg.value("done", false))
                {
                    // Footer — all chunks received
                    if (out_)
                    {
                        std::fclose(out_);
                        out_ = nullptr;
                    }
                    finish(totalRead_ == remoteSize_);
                }
            }

        void handleBinary(const rtc::binary& data)
            {
                if (!out_)
                    return;
                std::fwrite(data.data(), 1, data.size(), out_);
                totalRead_ += data.size();
                if (progressCb_)
                    progressCb_(totalRead_, remoteSize_);
            }

        void finish(bool success)
            {
                bool expected = false;
                if (!finished_.compare_exchange_strong(expected, true))
                    return;

                if (out_)
                {
                    std::fclose(out_);
                    out_ = nullptr;
                }

                if (success)
                    std::rename(partPath_.c_str(), localPath_.c_str());
                else
                    std::remove(partPath_.c_str());

                // Marshal back to main thread for UI/file open
                Fl::awake([this, success](void*)
                    {
                        if (doneCb_)
                            doneCb_(success);
                    }, nullptr);
            }

        std::string localPath_, partPath_;
        std::function<void(uint64_t,uint64_t)> progressCb_;
        std::function<void(bool)> doneCb_;
        FILE* out_ = nullptr;
        uint64_t remoteSize_ = 0;
        uint64_t totalRead_ = 0;
        std::atomic<bool> finished_{false};
    };

}  // namespace mrv
