
#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvWebRTCManager.h"

#include <tlCore/Path.h>

namespace mrv
{

    // Opens a "file-transfer" DataChannel to a specific peer and
    // reassembles the incoming chunks into a local file.
    class FileTransferClient
    {
    public:
        FileTransferClient(WebRTCManager& manager,
                           const std::string& peerId);

        void downloadFile(
            const tl::file::Path& remotePath,
            const tl::file::Path& localPath,
            std::function<void(bool& aborted,
                               const std::string& title,
                               uint64_t done,uint64_t total)> progressCb);

    private:
        void handleText(const std::string& text);
        void handleBinary(const rtc::binary& data);
        void finish(bool success);

        WebRTCManager& manager_;
        std::string peerId_;
        tl::file::Path localPath_, remotePath_;
        std::string partPath_;
        std::function<void(bool& aborted,
                           const std::string& title,
                           uint64_t,uint64_t)> progressCb_;
        std::function<void(bool)> doneCb_;
        FILE* out_ = nullptr;
        uint64_t remoteSize_ = 0;
        uint64_t totalRead_ = 0;
        std::atomic<bool> finished_{false};
    };

}  // namespace mrv
