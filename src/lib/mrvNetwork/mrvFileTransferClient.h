#include "mrvNetwork/mrvFileChunk.h"
#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvWebRTCManager.h"

#include <tlCore/Path.h>

#include <deque>
#include <vector>

namespace mrv
{

    // Opens a "file-transfer" DataChannel to a specific peer and
    // reassembles the incoming chunks into a local file.
    //
    // Image sequences (e.g. OpenEXR) are requested one whole file per
    // frame over an *ordered* channel — each frame is a self-contained
    // request/response before the next frame is requested.
    //
    // A single non-sequence file (e.g. a movie) is instead requested over
    // an *unordered* channel and streamed as many chunks that may arrive
    // out of order. Each chunk carries its destination byte offset (see
    // mrvFileChunk.h) so it can be written to the right place in the
    // output file regardless of arrival order, which is also the
    // foundation a future P2P / Read-Once-Send-Many-with-backfill scheme
    // would build on.
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
                               uint64_t done,uint64_t total) > progressCb,
            std::function<void(bool success,
                               const std::vector<std::string>& failedPaths) > doneCb);

        static void abortAwakeCB(void* data);

    private:
        //! Struct used for Fl::awake to cleanly exit from a datachannels'
        //! thread.
        struct AbortContext
        {
            FileTransferClient* self;
            std::shared_ptr<rtc::DataChannel> dc;
        };

        //! WebRTC datachannel handlers for text and binary data.
        void handleText(const std::string& text);
        void handleBinary(const rtc::binary& data);

        //! Handles a file error.
        void handleFileError(const std::string& err);

        //! Called on success or failure for cleanup.
        void finish(bool success);

        //! Helper method to process the queue
        void requestNextFile(std::shared_ptr<rtc::DataChannel> dc);

        //! Writes one offset-tagged chunk (payload only, header already
        //! stripped) to out_ at the given offset, updating counters,
        //! progress, and abort handling.
        void writeChunk(uint64_t offset, const std::byte* payload,
                        size_t payloadSize);

        // Finalizes the file currently being received (close + rename)
        // and moves on to the next queued file, if any.
        void completeCurrentFile();

        // Common completion check used by both the "done" control message
        // and data-chunk arrival, since on an unordered channel either one
        // may be the last to arrive.
        void checkComplete();

        // WebRTC handlers
        WebRTCManager& manager_;
        std::shared_ptr<rtc::DataChannel> dc_;
        std::string peerId_;

        // Local and remote paths (we use paths instead of strings to know the
        // list of frame numbers in sequences).
        tl::file::Path localPath_, remotePath_;
        std::string partPath_;

        // Function callbacks
        std::function<void(bool& aborted,
                           const std::string& title,
                           uint64_t,uint64_t)> progressCb_;

        std::function<void(bool, const std::vector<std::string>&)> doneCb_;

        // File writing variables.x
        FILE* out_ = nullptr;
        uint64_t remoteSize_ = 0;
        uint64_t totalRead_ = 0;
        std::atomic<bool> finished_{false};

        // parallel to pendingRemotePaths_/pendingLocalPaths_
        std::deque<bool>        pendingOptional_;
        bool                     currentIsOptional_ = false;

        // remote paths that failed but were skipped
        std::vector<std::string> failedPaths_;

        // Queues used for transferring multiple files.
        std::deque<std::string> pendingRemotePaths_;
        std::deque<std::string> pendingLocalPaths_;

        // Current files begin transferred
        std::string currentRemotePath_;
        std::string currentLocalPath_;
        std::string currentPartPath_;

        // --- Unordered-chunk reassembly state (movie transfers) ---

        // Bytes actually written for the file currently in flight. Used,
        // together with doneReceived_, to decide when a file is truly
        // complete instead of trusting the arrival order of the "done"
        // control message.
        uint64_t receivedBytes_ = 0;

        // Set once the "done" footer for the current file has been seen.
        // On an unordered channel this can arrive before the last data
        // chunk(s), so it only triggers completion once receivedBytes_
        // also reaches remoteSize_.
        bool doneReceived_ = false;

        // Binary chunks that arrive before the "size" header has opened
        // out_ (possible once the channel is unordered) are buffered here
        // and flushed once the file is open.
        std::vector<std::vector<std::byte>> pendingChunks_;

        bool otioExpanded_ = false;
        void expandOtioReferences(const tl::file::Path& remoteOtioPath,
                                  const std::string& localOtioPath);
    };

}  // namespace mrv
