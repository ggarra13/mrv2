#include "mrvNetwork/mrvFileTransferServer.h"
#include "mrvNetwork/mrvFileChunk.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

#include <tlCore/StringFormat.h>

#include <algorithm>
#include <condition_variable>
#include <thread>
#include <vector>

namespace
{
    const char* kModule = "sfts";

    // 8 MB high-water mark for per-subscriber backpressure.
    const size_t kMaxBufferedAmount = 8 * 1024 * 1024;
    const size_t kLowWaterMark = 4 * 1024 * 1024; // Wakes up at 4MB

    void sendError(std::shared_ptr<rtc::DataChannel> dc, const std::string& msg)
    {
        nlohmann::json err;
        err["error"] = msg;
        dc->send(err.dump());
        // NOTE: no dc->close() here anymore. This channel is reused for a
        // whole sequence of file requests (see FileTransferClient::
        // requestNextFile), so one missing path shouldn't tear down the
        // transport. Whether the channel closes after an error is now the
        // client's call: it closes dc_ itself for a required-file failure,
        // and keeps requesting on the same dc_ for an optional one.
    }

    void sendHeader(std::shared_ptr<rtc::DataChannel> dc,
                    const std::string& path, uint64_t size)
    {
        nlohmann::json header;
        header["size"] = size;
        header["path"] = path;
        dc->send(header.dump());
    }

    void sendDone(std::shared_ptr<rtc::DataChannel> dc)
    {
        nlohmann::json footer;
        footer["done"] = true;
        dc->send(footer.dump());
    }

    // Blocks until there's room to send more on `dc` without exceeding
    // kMaxBufferedAmount, or bails out (returns false) if the channel
    // closes or genuinely stalls. Polls in short increments rather than
    // relying on a single long wait_for: if onBufferedAmountLow doesn't
    // fire promptly for whatever reason, we still re-check the real
    // bufferedAmount() every 20ms instead of only after a full 5s
    // timeout, so a healthy peer that's actually draining fine never
    // visibly pauses. The notify still short-circuits each poll
    // immediately when it does fire.
    bool waitForRoom(std::shared_ptr<rtc::DataChannel> dc,
                     std::mutex& mtx, std::condition_variable& cv)
    {
        constexpr auto kPollInterval = std::chrono::milliseconds(20);
        constexpr auto kStallTimeout = std::chrono::seconds(5);

        const auto deadline = std::chrono::steady_clock::now() + kStallTimeout;

        std::unique_lock<std::mutex> lock(mtx);
        while (dc->isOpen() && dc->bufferedAmount() >= kLowWaterMark)
        {
            if (std::chrono::steady_clock::now() >= deadline)
                return false;
            cv.wait_for(lock, kPollInterval);
        }
        return dc->isOpen();
    }
}

namespace mrv
{

    // One requester attached to a Session, either as the original
    // request that started it or as a live/backfill joiner.
    struct FileTransferServer::Subscriber
    {
        std::shared_ptr<rtc::DataChannel> dc;

        // Byte offset the leader's read cursor was at when this
        // subscriber attached. 0 for the request that started the
        // session (it gets everything, live, from the start). Anything
        // > 0 means this subscriber needs [0, joinOffset) backfilled
        // after the leader's read finishes.
        uint64_t joinOffset = 0;

        // Max message size for this subscriber.
        size_t maxMessageSize = 1024 * 1024;

        // Backpressure primitives for this subscriber's DataChannel.
        // Shared (not owned) so the onBufferedAmountLow callback
        // registered on dc and the waiting code in runSession() refer to
        // the same instance regardless of how many times Subscriber is
        // copied around (e.g. when snapshotting the subscriber list).
        std::shared_ptr<std::mutex> bufMtx = std::make_shared<std::mutex>();
        std::shared_ptr<std::condition_variable> bufCv =
            std::make_shared<std::condition_variable>();
    };

    // Bookkeeping for one in-flight "read this path once" pass. Holds no
    // file content — only which peers are subscribed and where each one
    // joined.
    struct FileTransferServer::Session
    {
        std::string path;

        std::mutex mtx;
        std::vector<Subscriber> subscribers;   // protected by mtx
        uint64_t offset = 0;                   // bytes read so far, protected by mtx
        uint64_t size = 0;                     // protected by mtx
        bool headerSent = false;               // protected by mtx
    };

    std::shared_ptr<FileTransferServer>
    FileTransferServer::create(WebRTCManager& manager)
    {
        auto self = std::shared_ptr<FileTransferServer>(new FileTransferServer());
        self->init(manager);
        return self;
    }

    void FileTransferServer::init(WebRTCManager& manager)
    {
        std::weak_ptr<FileTransferServer> weakSelf = weak_from_this();
        manager.onExtraDataChannel =
            [weakSelf](const std::string& peerId, std::shared_ptr<rtc::DataChannel> dc)
                {
                    auto self = weakSelf.lock();
                    if (!self) return;
                    if (dc->label() == "file-transfer")
                        self->handleRequest(peerId, dc);
                };
    }

    void FileTransferServer::handleRequest(const std::string& peerId,
                                           std::shared_ptr<rtc::DataChannel> dc)
    {
        std::weak_ptr<FileTransferServer> weakSelf = weak_from_this();

        dc->onMessage(
            [weakSelf, dc](rtc::message_variant msg)
                {
                    auto self = weakSelf.lock();
                    if (!self)
                        return;

                    if (!std::holds_alternative<std::string>(msg))
                        return;

                    nlohmann::json req = nlohmann::json::parse(
                        std::get<std::string>(msg));
                    std::string path = req["path"];

                    std::shared_ptr<Session> session;
                    bool isNew = false;
                    {
                        std::lock_guard<std::mutex> lock(self->sessionsMutex_);
                        auto it = self->sessions_.find(path);
                        if (it != self->sessions_.end())
                        {
                            session = it->second;
                        }
                        else
                        {
                            session = std::make_shared<Session>();
                            session->path = path;
                            self->sessions_[path] = session;
                            isNew = true;
                        }

                        // Register this subscriber and, if we joined an
                        // in-flight session whose header has already gone
                        // out to whoever was subscribed at that point,
                        // send this subscriber its own header right now.
                        // (Every DataChannel needs its own "size"
                        // message — it's not something that can be
                        // broadcast once.) If the header hasn't been sent
                        // yet, we don't need to do anything here: we're
                        // already in the subscriber list, so the leader's
                        // own header broadcast (see runSession) will
                        // include us.
                        bool sendHeaderNow = false;
                        uint64_t knownSize = 0;
                        {
                            std::lock_guard<std::mutex> slock(session->mtx);
                            Subscriber sub;
                            sub.dc = dc;
                            sub.joinOffset = session->offset;
                            sub.maxMessageSize = dc->maxMessageSize();
                            sendHeaderNow = session->headerSent;
                            knownSize = session->size;

                            dc->setBufferedAmountLowThreshold(kLowWaterMark);
                            auto bufMtx = sub.bufMtx;
                            auto bufCv = sub.bufCv;
                            dc->onBufferedAmountLow([bufMtx, bufCv]() {
                                std::lock_guard<std::mutex> lock(*bufMtx);
                                bufCv->notify_one();
                            });

                            session->subscribers.push_back(std::move(sub));
                        }
                        if (sendHeaderNow)
                            sendHeader(dc, path, knownSize);
                    }

                    if (isNew)
                    {
                        std::thread([weakSelf, session]()
                            {
                                auto self = weakSelf.lock();
                                if (!self) return;
                                self->runSession(session);
                            }).detach();
                    }
                });
    }

    void FileTransferServer::runSession(std::shared_ptr<Session> session)
    {
        const std::string& path = session->path;

        FILE* f = std::fopen(path.c_str(), "rb");
        if (!f)
        {
            std::lock_guard<std::mutex> lock(sessionsMutex_);
            std::lock_guard<std::mutex> slock(session->mtx);
            sessions_.erase(path);
            for (auto& sub : session->subscribers)
                sendError(sub.dc, "File not found: " + path);
            return;
        }

        std::fseek(f, 0, SEEK_END);
        const uint64_t size = static_cast<uint64_t>(std::ftell(f));
        std::fseek(f, 0, SEEK_SET);

        // Send the header to whoever raced in and subscribed before we
        // got here (the request that created the session, plus any
        // near-simultaneous joiners), then mark it sent so any later
        // joiner gets one directly from handleRequest instead.
        {
            std::vector<Subscriber> initial;
            {
                std::lock_guard<std::mutex> lock(session->mtx);
                session->size = size;
                session->headerSent = true;
                initial = session->subscribers;
            }
            for (auto& sub : initial)
                sendHeader(sub.dc, path, size);
        }

        // Pick one chunk size for the whole session, based on whoever
        // started it. Fan-out doesn't require every subscriber to agree
        // on a chunk size — it only affects framing — so a late joiner
        // with a smaller/larger negotiated max message size than the
        // leader would be a rare mismatch to handle specially, and isn't
        // handled here; in practice peers in the same session use the
        // same DataChannel configuration.
        size_t maxMsgSize = 0;
        {
            std::lock_guard<std::mutex> lock(session->mtx);
            if (!session->subscribers.empty())
                maxMsgSize = session->subscribers.front().dc->maxMessageSize();
        }

        // If 0, treat as unbounded (defaulting to our 1MB preference)
        if (maxMsgSize == 0) maxMsgSize = 1024 * 1024;

        size_t kChunkSize = std::min<size_t>(1024 * 1024, maxMsgSize);
        kChunkSize = (kChunkSize > kChunkHeaderSize)
                         ? kChunkSize - kChunkHeaderSize
                         : 1;

        std::vector<std::byte> buf(kChunkHeaderSize + kChunkSize);
        uint64_t offset = 0;

        while (!std::feof(f))
        {
            size_t n = std::fread(buf.data() + kChunkHeaderSize, 1,
                                  kChunkSize, f);
            if (n == 0)
                break;

            packChunkOffset(offset, buf.data());

            // Snapshot who's currently live. New joiners that attach
            // after this snapshot but before the next one will simply be
            // included starting next chunk — their joinOffset already
            // recorded whatever `offset` was when they attached, so
            // they'll get backfilled for anything they missed in
            // between.
            std::vector<Subscriber> targets;
            {
                std::lock_guard<std::mutex> lock(session->mtx);
                targets = session->subscribers;
            }

            for (auto& sub : targets)
            {
                if (!sub.dc->isOpen())
                {
                    // Dropped/closed on its own — remove it so it's not
                    // considered for "done" or backfill later.
                    std::lock_guard<std::mutex> lock(session->mtx);
                    auto& subs = session->subscribers;
                    subs.erase(std::remove_if(subs.begin(), subs.end(),
                        [&](const Subscriber& s) { return s.dc == sub.dc; }),
                        subs.end());
                    continue;
                }

                if (sub.dc->bufferedAmount() >= kMaxBufferedAmount)
                {
                    if (!waitForRoom(sub.dc, *sub.bufMtx, *sub.bufCv) ||
                        !sub.dc->isOpen())
                    {
                        LOG_ERROR("Subscriber stalled or disconnected, "
                                  "dropping from live session: " + path);
                        std::lock_guard<std::mutex> lock(session->mtx);
                        auto& subs = session->subscribers;
                        subs.erase(std::remove_if(subs.begin(), subs.end(),
                            [&](const Subscriber& s) { return s.dc == sub.dc; }),
                            subs.end());
                        continue;
                    }
                }

                try
                {
                    // Fall back to 1MB if the subscriber's limit is reporting as unbounded (0)
                    size_t subMaxMsgSize = sub.maxMessageSize;
                    if (subMaxMsgSize == 0) subMaxMsgSize = 1024 * 1024;

                    const size_t totalChunkSize = kChunkHeaderSize + n;

                    // Fast Path: If the overall chunk fits, send it directly
                    if (totalChunkSize <= subMaxMsgSize)
                    {
                        sub.dc->send(buf.data(), totalChunkSize);
                    }
                    // Slow Path: Chunk is too big for this subscriber. Split and send in slices
                    else
                    {
                        // Determine the maximum payload portion we can pack alongside the header
                        const size_t subChunkLimit = (subMaxMsgSize > kChunkHeaderSize)
                                                     ? subMaxMsgSize - kChunkHeaderSize
                                                     : 1;

                        std::vector<std::byte> scratch;
                        size_t bytesSent = 0;

                        while (bytesSent < n)
                        {
                            const size_t chunkPayloadSize = std::min(subChunkLimit, n - bytesSent);

                            // Resize our scratch buffer to hold the header + this slice's payload.
                            // Using resize() on a persistent buffer avoids repeated heap allocations.
                            scratch.resize(kChunkHeaderSize + chunkPayloadSize);

                            // Compute and pack the absolute file offset for this slice
                            const uint64_t subOffset = offset + bytesSent;
                            packChunkOffset(subOffset, scratch.data());

                            // Copy the payload slice from our main read buffer to the scratch buffer
                            std::copy(buf.begin() + kChunkHeaderSize + bytesSent,
                                      buf.begin() + kChunkHeaderSize + bytesSent + chunkPayloadSize,
                                      scratch.begin() + kChunkHeaderSize);

                            // Transmit the sub-chunk
                            sub.dc->send(scratch.data(), scratch.size());

                            bytesSent += chunkPayloadSize;
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Send failed for subscriber, dropping from live session: " +
                              path + " (" + e.what() + ")");
                    std::lock_guard<std::mutex> lock(session->mtx);
                    auto& subs = session->subscribers;
                    subs.erase(std::remove_if(subs.begin(), subs.end(),
                                              [&](const Subscriber& s) {
                                                  return s.dc == sub.dc;
                                              }), subs.end());
                }
            }

            offset += n;
            {
                std::lock_guard<std::mutex> lock(session->mtx);
                session->offset = offset;
            }
        }
        std::fclose(f);

        // Leader's read is done. Tell every subscriber still attached
        // that no more live chunks are coming; subscribers that joined
        // late are missing their prefix, which each gets in a small
        // dedicated backfill read below. Sending "done" to them now
        // (rather than after their backfill) is safe: the client only
        // finalizes a file once its received-byte total reaches the size
        // it was told, regardless of which message — "done" or the last
        // data chunk — happens to arrive last.
        std::vector<Subscriber> lateJoiners;
        {
            std::lock_guard<std::mutex> lock(sessionsMutex_);
            std::lock_guard<std::mutex> slock(session->mtx);
            sessions_.erase(path);
            for (auto& sub : session->subscribers)
            {
                sendDone(sub.dc);
                if (sub.joinOffset > 0)
                    lateJoiners.push_back(sub);
            }
        }

        for (auto& sub : lateJoiners)
            backfillSubscriber(sub.dc, path, sub.joinOffset);
    }

    void FileTransferServer::backfillSubscriber(
        std::shared_ptr<rtc::DataChannel> dc, const std::string& path,
        uint64_t uptoOffset)
    {
        if (!dc->isOpen())
            return;

        FILE* f = std::fopen(path.c_str(), "rb");
        if (!f)
        {
            sendError(dc, "File not found during backfill: " + path);
            return;
        }

        size_t maxMsgSize = dc->maxMessageSize();

        // If 0, treat as unbounded (defaulting to our 1MB preference)
        if (maxMsgSize == 0) maxMsgSize = 1024 * 1024;

        size_t kChunkSize = std::min<size_t>(1024 * 1024, maxMsgSize);

        kChunkSize = (kChunkSize > kChunkHeaderSize)
                         ? kChunkSize - kChunkHeaderSize
                         : 1;

        auto mtx = std::make_shared<std::mutex>();
        auto cv = std::make_shared<std::condition_variable>();

        dc->setBufferedAmountLowThreshold(kLowWaterMark);
        dc->onBufferedAmountLow([mtx, cv]() {
            std::lock_guard<std::mutex> lock(*mtx); // Safely locked
            cv->notify_one();
        });

        std::vector<std::byte> buf(kChunkHeaderSize + kChunkSize);
        uint64_t offset = 0;

        while (offset < uptoOffset && !std::feof(f))
        {
            const size_t remaining =
                static_cast<size_t>(std::min<uint64_t>(
                    kChunkSize, uptoOffset - offset));
            size_t n = std::fread(buf.data() + kChunkHeaderSize, 1,
                                  remaining, f);
            if (n == 0)
                break;

            if (!dc->isOpen())
                break;

            if (dc->bufferedAmount() >= kMaxBufferedAmount)
            {
                if (!waitForRoom(dc, *mtx, *cv) || !dc->isOpen())
                {
                    LOG_ERROR("Backfill stalled or disconnected: " + path);
                    break;
                }
            }

            packChunkOffset(offset, buf.data());
            dc->send(buf.data(), kChunkHeaderSize + n);
            offset += n;
        }
        std::fclose(f);
    }

}  // namespace mrv
