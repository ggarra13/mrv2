
#include "mrvNetwork/mrvFileTransferClient.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "ftc";
}


namespace mrv
{

    FileTransferClient::FileTransferClient(WebRTCManager& manager,
                                           const std::string& peerId) :
        manager_(manager),
        peerId_(peerId)
    {
    }

    void FileTransferClient::downloadFile(const tl::file::Path& remotePath,
                                          const tl::file::Path& localPath,
                                          std::function<void(
                                              bool& aborted,
                                              const std::string& title,
                                               uint64_t,uint64_t)> progressCb)
    {
        remotePath_ = remotePath;
        localPath_ = localPath;
        partPath_ = localPath.get() + ".part";
        progressCb_ = std::move(progressCb);

        auto peer = manager_.getClient(peerId_);
        if (!peer)
        {
            return;
        }

        rtc::DataChannelInit init;
        init.reliability.unordered = false;

        auto dc = peer->peerConnection->createDataChannel(
            "file-transfer", init);

        dc->onOpen([dc, remotePath]()
            {
                Message req;
                req["path"] = remotePath.get();
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

    void FileTransferClient::handleText(const std::string& text)
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

    void FileTransferClient::handleBinary(const rtc::binary& data)
    {
        if (!out_)
            return;
        std::fwrite(data.data(), 1, data.size(), out_);
        totalRead_ += data.size();

        bool aborted = false;
        if (progressCb_)
        {
            const std::string title =
                tl::string::Format(_("Downloading {0}...")).
                arg(remotePath_.get());
            progressCb_(aborted, title, totalRead_, remoteSize_);
            if (aborted)
            {
                finish(false);
            }
        }
    }

    void FileTransferClient::finish(bool success)
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
        {
            std::string newName = localPath_.get();
            std::rename(partPath_.c_str(), newName.c_str());

            bool aborted = false;
            progressCb_(aborted, "", remoteSize_, remoteSize_);
        }
        else
        {
            std::remove(partPath_.c_str());
        }

        // // Marshal back to main thread for UI/file open
        // Fl::awake([this, success](void*)
        //     {
        //         if (doneCb_)
        //             doneCb_(success);
        //     }, nullptr);
    }

}  // namespace mrv
