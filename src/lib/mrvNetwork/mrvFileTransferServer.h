#include "mrvNetwork/mrvWebRTCManager.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace mrv
{

    // FileTransferServer.h
    // Runs on every peer at startup.
    // Responds to incoming "file-transfer" DataChannels by reading the
    // requested file off local disk and sending it in offset-tagged
    // chunks (see mrvFileChunk.h). Whether those chunks land in order or
    // not is entirely a property of the requesting peer's DataChannel
    // (ordered for image-sequence frames, unordered for movie files) —
    // this server doesn't need to know or care which mode it's in.
    //
    // Single-Read Multiplexing (Read Once, Send Many):
    //
    // If a second (third, ...) request for a path arrives while a read
    // of that same path is already under way, it doesn't trigger a
    // second full read from disk. Instead it joins the in-flight
    // "Session" as a subscriber and starts receiving chunks live from
    // wherever the ongoing read currently is. Once that read finishes,
    // any subscriber that joined late is backfilled with just the byte
    // range it missed — a second, much smaller read bounded to
    // [0, joinOffset) rather than a second full-file read.
    //
    // No file content is cached in memory to make this work: a Session
    // only tracks bookkeeping (which peers are subscribed, and at what
    // offset each one joined). Chunks are read once, fanned out live to
    // whoever is currently subscribed, and then discarded — exactly one
    // full read per file per "burst" of concurrent requests, plus one
    // small bounded read per straggler.
    //
    // A request that arrives after a Session has already finished (no
    // overlap at all) simply starts a brand new Session, i.e. a plain
    // single-reader full read — the degenerate case when there's no
    // concurrency to multiplex.
    class FileTransferServer
    {
    public:
        explicit FileTransferServer(WebRTCManager& manager);

    private:
        struct Subscriber;
        struct Session;

        void handleRequest(const std::string& peerId,
                           std::shared_ptr<rtc::DataChannel> dc);

        // Reads `session->path` once, start to finish, fanning each chunk
        // out live to whichever subscribers are currently attached to
        // the session. Runs on its own thread; the thread that creates a
        // brand-new Session is responsible for spawning it.
        void runSession(std::shared_ptr<Session> session);

        // Streams just the byte range [0, uptoOffset) of `path` to a
        // single late-joining subscriber, to fill in the prefix it
        // missed while attaching mid-stream. No "size" header is sent —
        // the subscriber already has one from when it joined the live
        // session — and no "done" is sent either, since the client only
        // finalizes once its received-byte total reaches the size it was
        // already told, regardless of which message got it there.
        void backfillSubscriber(std::shared_ptr<rtc::DataChannel> dc,
                                const std::string& path,
                                uint64_t uptoOffset);

        // path -> the Session currently reading it, if any. A path is
        // only present here while a leader read is actively in flight;
        // it's removed as soon as that read finishes (backfills for
        // stragglers happen afterward, outside the registry).
        std::mutex sessionsMutex_;
        std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
    };

}  // namespace mrv
