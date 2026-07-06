
#include "mrvNetwork/mrvWebRTCManager.h"

namespace mrv
{

    // FileTransferServer.h
    // Runs on every peer at startup.
    // Responds to incoming "file-transfer" DataChannels by reading the
    // requested file off local disk and sending it in chunks.
    class FileTransferServer
    {
    public:
        explicit FileTransferServer(WebRTCManager& manager);

    private:
        void handleRequest(const std::string& peerId,
                           std::shared_ptr<rtc::DataChannel> dc);
    };

}  // namespace mrv
