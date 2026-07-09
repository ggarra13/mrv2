#include "mrvNetwork/mrvFileTransferClient.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

#include <tlCore/StringFormat.h>


namespace
{
    const char* kModule = "wftc";
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
                                              uint64_t,uint64_t) > progressCb,
                                          std::function<void(bool) > doneCb)
    {
        remotePath_ = remotePath;
        localPath_ = localPath;
        partPath_ = localPath.get() + ".part";
        progressCb_ = std::move(progressCb);
        doneCb_ = std::move(doneCb);

        // Populate the download queues
        auto frames = remotePath.getFrames();
        const bool isSequence = remotePath.isSequence() && frames.has_value();
        if (isSequence)
        {
            math::Int64Range range = frames.value();
            const bool listdir = true;
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
            LOG_ERROR("No peerId " << peerId_);
            return;
        }

        rtc::DataChannelInit init;
        // Image sequences: each frame is requested and fully received
        // before the next is asked for, so keep the channel ordered.
        // Movie files: a single file is streamed as many chunks that we
        // want delivered as fast as possible without head-of-line
        // blocking, so open the channel unordered. Every chunk carries
        // its destination byte offset (mrvFileChunk.h) so it can be
        // written to the right place regardless of arrival order.
        init.reliability.unordered = !isSequence;

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

        dc->onClosed([this]() {
            finish(false);
        });
        dc->onError([this](std::string err) {
            LOG_ERROR(err);
            finish(false);
        });
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
                return;
            }

            // On an unordered channel, chunks may have already arrived
            // before this header did. Replay them now that the file is
            // open.
            for (const auto& chunk : pendingChunks_)
            {
                if (chunk.size() < kChunkHeaderSize)
                    continue;
                const uint64_t offset = unpackChunkOffset(chunk.data());
                writeChunk(offset, chunk.data() + kChunkHeaderSize,
                           chunk.size() - kChunkHeaderSize);
            }
            pendingChunks_.clear();

            checkComplete();
        }
        else if (msg.value("done", false))
        {
            // Footer received. On an ordered (sequence) channel every
            // data chunk necessarily arrived first, so this completes the
            // file immediately. On an unordered (movie) channel this can
            // race ahead of the last chunk(s), so we only finalize once
            // checkComplete() sees receivedBytes_ has caught up.
            doneReceived_ = true;
            checkComplete();
        }
    }

    void FileTransferClient::handleBinary(const rtc::binary& data)
    {
        if (data.size() < kChunkHeaderSize)
        {
            LOG_ERROR("Malformed chunk (smaller than the offset header)");
            return;
        }

        if (!out_)
        {
            // "size" hasn't opened the output file yet — can happen on an
            // unordered channel where control and data messages can race.
            // Buffer the raw message (header included) and replay it once
            // the file is open.
            pendingChunks_.emplace_back(data.begin(), data.end());
            return;
        }

        const uint64_t offset = unpackChunkOffset(data.data());
        writeChunk(offset, data.data() + kChunkHeaderSize,
                  data.size() - kChunkHeaderSize);
    }

    void FileTransferClient::writeChunk(uint64_t offset,
                                        const std::byte* payload,
                                        size_t payloadSize)
    {
        std::fseek(out_, static_cast<long>(offset), SEEK_SET);
        std::fwrite(payload, 1, payloadSize, out_);
        totalRead_ += payloadSize;
        receivedBytes_ += payloadSize;

        const file::Path path(currentRemotePath_);
        const std::string title =
            tl::string::Format(_("Downloading {0}...")).arg(path.get());

        bool aborted = false;
        if (progressCb_)
        {
            progressCb_(aborted, title, totalRead_, remoteSize_);
            if (aborted)
            {
                // Sever the connection to stop incoming traffic
                if (dc_) {
                    dc_->close();
                }

                // Clear pending queues so the sequence fully stops
                pendingRemotePaths_.clear();
                pendingLocalPaths_.clear();

                // Clean up local state
                finish(false);
                return;
            }
        }

        checkComplete();
    }

    void FileTransferClient::checkComplete()
    {
        if (out_ && doneReceived_ && receivedBytes_ >= remoteSize_)
            completeCurrentFile();
    }

    void FileTransferClient::completeCurrentFile()
    {
        if (out_)
        {
            std::fclose(out_);
            out_ = nullptr;
        }

        // Finalize this specific file
        std::rename(currentPartPath_.c_str(), currentLocalPath_.c_str());

        doneReceived_ = false;
        receivedBytes_ = 0;
        pendingChunks_.clear();

        auto peer = manager_.getClient(peerId_);
        if (peer)
        {
            requestNextFile(dc_);
        }
    }

    void FileTransferClient::finish(bool success)
    {
        bool expected = false;
        if (!finished_.compare_exchange_strong(expected, true))
        {
            return;
        }

        if (out_)
        {
            std::fclose(out_);
            out_ = nullptr;
        }

        if (success)
        {
            std::rename(currentPartPath_.c_str(), currentLocalPath_.c_str());
        }
        else
        {
            std::remove(currentPartPath_.c_str());
        }

        if (doneCb_)
            doneCb_(success);
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
        currentRemotePath_ = pendingRemotePaths_.front();
        pendingRemotePaths_.pop_front();

        currentLocalPath_ = pendingLocalPaths_.front();
        pendingLocalPaths_.pop_front();

        currentPartPath_ = currentLocalPath_ + ".part";

        // Reset state for this specific file
        totalRead_ = 0;
        remoteSize_ = 0;
        receivedBytes_ = 0;
        doneReceived_ = false;
        pendingChunks_.clear();

        // Ask the server for it
        Message req;
        req["path"] = currentRemotePath_;
        dc->send(req.dump());
    }

}  // namespace mrv
