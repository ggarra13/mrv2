
#include "mrvNetwork/mrvFileTransferServer.h"
#include "mrvNetwork/mrvWebRTCManager.h"
#include "mrvNetwork/mrvMessage.h"

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

        // Send chunks
        const size_t kChunkSize = 256 * 1024;
        std::vector<char> buf(kChunkSize);
        while (!std::feof(f))
        {
            size_t n = std::fread(buf.data(), 1, kChunkSize, f);
            if (n == 0)
                break;

            // Check if the client closed the connection or aborted
            if (!dc->isOpen()) 
                break;
            
            bool success = dc->send(reinterpret_cast<const std::byte*>(buf.data()), n);
            if (!success)
                break;
        }
        std::fclose(f);

        // Send footer
        nlohmann::json footer;
        footer["done"] = true;
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
