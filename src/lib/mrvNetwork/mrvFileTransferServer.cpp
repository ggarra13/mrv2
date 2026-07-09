#include "mrvNetwork/mrvFileTransferServer.h"
#include "mrvNetwork/mrvFileChunk.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

#include <algorithm>
#include <condition_variable>
#include <thread>
#include <vector>

namespace
{
    const char* kModule = "sfts";

    // 8 MB high-water mark for per-subscriber backpressure.
    const size_t kMaxBufferedAmount = 8 * 1024 * 1024;

    void sendError(std::shared_ptr<rtc::DataChannel> dc, const std::string& msg)
    {
        nlohmann::json err;
        err["error"] = msg;
        dc->send(err.dump());
        dc->close();
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
    // closes or a peer stalls for too long. Mirrors the single-peer
    // backpressure logic from before, just parameterized per subscriber
    // so a slow peer's wait doesn't need a bespoke condition variable
    // wired up ad hoc at every call site.
    bool waitForRoom(std::shared_ptr<rtc::DataChannel> dc,
                     std::mutex& mtx, std::condition_variable& cv)
    {
        std::unique_lock<std::mutex> lock(mtx);
        return cv.wait_for(lock, std::chrono::seconds(5), [&]()
            {
                return !dc->isOpen() || dc->bufferedAmount() < kMaxBufferedAmount;
            });
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

    FileTransferServer::FileTransferServer(WebRTCManager& manager)
    {
        manager.onExtraDataChannel =
            [this](const std::string& peerId,
                   std::shared_ptr<rtc::DataChannel> dc)
                {
                    if (dc->label() == "file-transfer")
                        handleRequest(peerId, dc);
                };
    }

    void FileTransferServer::handleRequest(const std::string& peerId,
                                           std::shared_ptr<rtc::DataChannel> dc)
    {
        dc->onMessage(
            [this, dc](rtc::message_variant msg)
                {
                    if (!std::holds_alternative<std::string>(msg))
                        return;

                    nlohmann::json req = nlohmann::json::parse(
                        std::get<std::string>(msg));
                    std::string path = req["path"];

                    std::shared_ptr<Session> session;
                    bool isNew = false;
                    {
                        std::lock_guard<std::mutex> lock(sessionsMutex_);
                        auto it = sessions_.find(path);
                        if (it != sessions_.end())
                        {
                            session = it->second;
                        }
                        else
                        {
                            session = std::make_shared<Session>();
                            session->path = path;
                            sessions_[path] = session;
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
                            sendHeaderNow = session->headerSent;
                            knownSize = session->size;

                            dc->setBufferedAmountLowThreshold(kMaxBufferedAmount);
                            auto bufMtx = sub.bufMtx;
                            auto bufCv = sub.bufCv;
                            dc->onBufferedAmountLow([bufCv]() { bufCv->notify_one(); });

                            session->subscribers.push_back(std::move(sub));
                        }
                        if (sendHeaderNow)
                            sendHeader(dc, path, knownSize);
                    }

                    if (isNew)
                    {
                        std::thread([this, session]()
                            {
                                runSession(session);
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

                sub.dc->send(buf.data(), kChunkHeaderSize + n);
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
        size_t kChunkSize = std::min<size_t>(1024 * 1024, maxMsgSize);
        kChunkSize = (kChunkSize > kChunkHeaderSize)
                         ? kChunkSize - kChunkHeaderSize
                         : 1;

        auto mtx = std::make_shared<std::mutex>();
        auto cv = std::make_shared<std::condition_variable>();
        dc->onBufferedAmountLow([cv]() { cv->notify_one(); });
        dc->setBufferedAmountLowThreshold(kMaxBufferedAmount);

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
