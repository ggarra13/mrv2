
#include "mrvNetwork/mrvFileTransferServer.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "sfts";
}

namespace mrv
{
    static void sendFile(std::shared_ptr<rtc::DataChannel> dc,
                         const std::string& path)
    {
        FILE* f = std::fopen(path.c_str(), "rb");
        if (!f)
        {
            nlohmann::json err;
            err["error"] = "File not found: " + path;
            dc->send(err.dump());
            dc->close();
            return;
        }

        // Get size
        std::fseek(f, 0, SEEK_END);
        uint64_t size = static_cast<uint64_t>(std::ftell(f));
        std::fseek(f, 0, SEEK_SET);

        // Send header
        nlohmann::json header;
        header["size"] = size;
        header["path"] = path;
        dc->send(header.dump());

        // --- Backpressure machinery ---
        const size_t kMaxBufferedAmount = 1024 * 1024; // 1 MB high-water mark
        std::mutex mtx;
        std::condition_variable cv;

        // Fires whenever bufferedAmount() drops below the threshold set below.
        dc->onBufferedAmountLow(
            [&cv]()
                {
                    cv.notify_one();
                });
        dc->setBufferedAmountLowThreshold(kMaxBufferedAmount);

        // Blocks until there's room to send more without exceeding the
        // high-water mark. Also bails out (returns false) if the channel
        // closes while we're waiting.
        auto waitForRoom = [&]() -> bool
            {
                std::unique_lock<std::mutex> lock(mtx);
                return cv.wait_for(lock, std::chrono::seconds(5), [&]()
                    {
                        return !dc->isOpen() ||
                            dc->bufferedAmount() < kMaxBufferedAmount;
                    });
            };

        // Send chunks
        const size_t kChunkSize = 256 * 1024;
        std::vector<char> buf(kChunkSize);
        bool ok = true;

        while (!std::feof(f))
        {
            size_t n = std::fread(buf.data(), 1, kChunkSize, f);
            if (n == 0)
                break;

            if (!dc->isOpen())
            {
                ok = false;
                break;
            }

            // If the buffer is already over the high-water mark, wait for
            // onBufferedAmountLow to fire (or the peer to disappear) before
            // pushing more data in.
            if (dc->bufferedAmount() >= kMaxBufferedAmount)
            {
                if (!waitForRoom())
                {
                    LOG_ERROR("Timed out waiting for buffer to drain");
                    ok = false;
                    break;
                }
                if (!dc->isOpen())
                {
                    ok = false;
                    break;
                }
            }

            dc->send(
                reinterpret_cast<const std::byte*>(buf.data()), n);
        }
        std::fclose(f);

        // Only report success if every chunk actually went out. Sending
        // "done" on a truncated transfer causes the client to rename the
        // partial .part file into place as if it were complete.
        nlohmann::json footer;
        if (ok)
            footer["done"] = true;
        else
            footer["error"] = "Transfer failed or was interrupted: " + path;

        dc->send(footer.dump());
    }

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
            [dc](rtc::message_variant msg)
                {
                    if (!std::holds_alternative<std::string>(msg))
                        return;

                    nlohmann::json req = nlohmann::json::parse(
                        std::get<std::string>(msg));
                    std::string path = req["path"];

                    // Spin a thread so the DataChannel callback
                    // returns immediately.
                    std::thread([dc, path]()
                        {
                            sendFile(dc, path);
                        }).detach();
                });
    }

}  // namespace mrv
