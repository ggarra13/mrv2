
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
        
        // Populate the download queues
        auto frames = remotePath.getFrames();
        if (remotePath.isSequence() && frames.has_value())
        {
            math::Int64Range range = frames.value();
            const bool listdir = true; // or however your API handles this
            for (int64_t i = range.getMin(); i <= range.getMax(); ++i)
            {
                pendingRemotePaths_.push_back(remotePath.getFrame(i, listdir));
                pendingLocalPaths_.push_back(localPath.getFrame(i, listdir)); 
            }
        }
        else
        {
            pendingRemotePaths_.push_back(remotePath.get());
            pendingLocalPaths_.push_back(localPath.get());
        }
        
        auto peer = manager_.getClient(peerId_);
        if (!peer)
        {
            return;
        }

        rtc::DataChannelInit init;
        init.reliability.unordered = false;

        auto dc = dc_ = peer->peerConnection->createDataChannel(
            "file-transfer", init);

        dc->onOpen([this, dc]()
            {
                requestNextFile(dc);
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
            remoteSize_ = msg["size"];
            // Open the file using our updated currentPartPath_
            out_ = std::fopen(currentPartPath_.c_str(), "wb");
            if (!out_)
            {
                LOG_ERROR("Could not open " + currentPartPath_);
                finish(false);
            }
        }
        else if (msg.value("done", false))
        {
            // Footer — current file received
            if (out_)
            {
                std::fclose(out_);
                out_ = nullptr;
            }
            
            // Finalize this specific file
            std::rename(currentPartPath_.c_str(), currentLocalPath_.c_str());

            // Retrieve the DataChannel (assuming you can grab it from peerConnection or store it as a member variable `dc_`)
            auto peer = manager_.getClient(peerId_);
            if (peer) 
            {
                 // Find the existing data channel. Alternatively, make `dc_` a member of FileTransferClient
                 // For brevity, assuming you added std::shared_ptr<rtc::DataChannel> dc_ to your class:
                 requestNextFile(dc_); 
            }
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

    void FileTransferClient::requestNextFile(std::shared_ptr<rtc::DataChannel> dc)
    {
        if (pendingRemotePaths_.empty())
        {
            // All files are downloaded, now we can safely close out
            finish(true);
            dc->close();
            return;
        }

        // Pop the next file off the queues
        std::string remote = pendingRemotePaths_.front();
        pendingRemotePaths_.pop_front();
        
        currentLocalPath_ = pendingLocalPaths_.front();
        pendingLocalPaths_.pop_front();
        
        currentPartPath_ = currentLocalPath_ + ".part";
        
        // Reset state for this specific file
        totalRead_ = 0;
        remoteSize_ = 0;

        // Ask the server for it
        Message req;
        req["path"] = remote;
        dc->send(req.dump());
    }
    
}  // namespace mrv
