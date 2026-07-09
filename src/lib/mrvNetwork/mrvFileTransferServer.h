#include "mrvNetwork/mrvWebRTCManager.h"

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
    // Future extension point: to support Read-Once-Send-Many, this is
    // where a static registry of in-flight sendFile() sessions keyed by
    // path would live. A second request for a path already being read
    // would subscribe to (or be backfilled from) the existing session's
    // chunks instead of opening a second file descriptor — no wire-format
    // change would be needed, since chunks are already offset-tagged and
    // therefore safe to replay to a late joiner in any order.
    class FileTransferServer
    {
    public:
        explicit FileTransferServer(WebRTCManager& manager);

    private:
        void handleRequest(const std::string& peerId,
                           std::shared_ptr<rtc::DataChannel> dc);
    };

}  // namespace mrv
