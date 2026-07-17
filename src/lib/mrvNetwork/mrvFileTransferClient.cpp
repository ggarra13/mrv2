#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvNetwork/mrvFileTransferClient.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

#include <tlCore/StringFormat.h>

#include <FL/Fl.H>

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
                                              uint64_t,uint64_t) > progressCb,
                                          std::function<void(bool,
                                                             const std::vector<std::string>&) > doneCb)
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
                pendingOptional_.push_back(false);
            }
        }
        else
        {
            pendingRemotePaths_.push_back(remotePath.get());
            pendingLocalPaths_.push_back(localPath.get());
            pendingOptional_.push_back(false);
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
            if (!currentRemotePath_.empty())
                failedPaths_.push_back(currentRemotePath_);
            auto* ctx = new AbortContext{this, dc_};
            Fl::awake(&FileTransferClient::abortAwakeCB, ctx);
        });
        dc->onError([this](std::string err) {
            LOG_ERROR(err);
            if (!currentRemotePath_.empty())
                failedPaths_.push_back(currentRemotePath_);
            auto* ctx = new AbortContext{this, dc_};
            Fl::awake(&FileTransferClient::abortAwakeCB, ctx);
        });
    }

    void FileTransferClient::handleFileError(const std::string& err)
    {
        LOG_ERROR(err);

        if (out_)
        {
            std::fclose(out_);
            out_ = nullptr;
        }
        std::remove(currentPartPath_.c_str());

        if (currentIsOptional_)
        {
            // A referenced clip/audio/sequence frame the .otio pointed to
            // isn't there. Note it and keep going — the rest of the
            // timeline can still come down and be opened with this one
            // piece offline/missing, same as tlRender already tolerates
            // locally.
            failedPaths_.push_back(currentRemotePath_);

            doneReceived_ = false;
            receivedBytes_ = 0;
            pendingChunks_.clear();

            if (manager_.getClient(peerId_))
                requestNextFile(dc_);
            return;
        }

        // Required file (the .otio itself, or a plain movie/sequence
        // frame outside the otio-expansion path) — same hard-abort
        // behavior as before.
        finish(false);
    }

    void FileTransferClient::handleText(const std::string& text)
    {
        Message msg = nlohmann::json::parse(text);

        if (msg.contains("error"))
        {
            handleFileError(msg["error"].get<std::string>());
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

        const tl::file::Path path(currentRemotePath_);

        bool aborted = false;
        if (progressCb_)
        {
            const std::string title =
                tl::string::Format(_("Downloading {0}...")).arg(path.get());

            progressCb_(aborted, title, totalRead_, remoteSize_);
            if (aborted)
            {
                // Clear pending queues so the sequence fully stops
                pendingRemotePaths_.clear();
                pendingLocalPaths_.clear();
                pendingOptional_.clear();

                // We're still executing inside dc_'s own message-dispatch
                // stack (flushPendingMessages). Closing dc_ or destroying
                // `this` (via finish() -> doneCb_) here corrupts
                // libdatachannel's internal message queue. Defer real
                // teardown to the main thread via Fl::awake, after this
                // callback has returned.
                auto* ctx = new AbortContext{this, dc_};
                Fl::awake(&FileTransferClient::abortAwakeCB, ctx);
                return;
            }
        }

        checkComplete();
    }

    void FileTransferClient::abortAwakeCB(void* data)
    {
        std::unique_ptr<AbortContext> ctx(
            static_cast<AbortContext*>(data));
        if (ctx->dc)
            ctx->dc->close();
        ctx->self->finish(false);
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

        const tl::file::Path finishedRemote(currentRemotePath_);
        if (!otioExpanded_ && finishedRemote.getExtension() == ".otio")
        {
            otioExpanded_ = true;
            expandOtioReferences(finishedRemote, currentLocalPath_);
        }

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
            doneCb_(success, failedPaths_);
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

        currentIsOptional_ = pendingOptional_.front();
        pendingOptional_.pop_front();

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

    void FileTransferClient::expandOtioReferences(
        const tl::file::Path& remoteOtioPath,
        const std::string& localOtioPath)
    {
        otio::ErrorStatus err;
        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_file(localOtioPath, &err)));
        if (!timeline || otio::is_error(err))
        {
            LOG_ERROR("Could not parse downloaded .otio: " + localOtioPath);
            return;
        }

        // The server-side directory the referenced media is relative to —
        // we already know it, since we're the ones who chose remotePath_.
        const std::string remoteBaseDir = remoteOtioPath.getDirectory();
        const std::string localBaseDir  = tl::file::Path(localOtioPath).getDirectory();

        for (const auto& mediaPath : getOtioTimelinePaths(timeline, remoteBaseDir))
        {
            // Re-anchor under the local download dir, preserving whatever
            // relative sub-structure the .otio itself used, so its
            // internal (relative) references stay valid after download.
            std::string localMedia = mediaPath.get();
            if (localMedia.rfind(remoteBaseDir, 0) == 0)
                localMedia = localBaseDir + localMedia.substr(remoteBaseDir.size());
            else
                localMedia = localBaseDir + mediaPath.getFileName();

            if (mediaPath.isSequence() && mediaPath.getFrames().has_value())
            {
                const auto range = mediaPath.getFrames().value();
                const tl::file::Path localMediaPath(localMedia);
                const bool listdir = true;
                for (int64_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    pendingRemotePaths_.push_back(mediaPath.getFrame(i, listdir));
                    pendingLocalPaths_.push_back(localMediaPath.getFrame(i, listdir));
                    pendingOptional_.push_back(true);
                }
            }
            else
            {
                pendingRemotePaths_.push_back(mediaPath.get());
                pendingLocalPaths_.push_back(localMedia);
                pendingOptional_.push_back(true);
            }
        }
    }

}  // namespace mrv
