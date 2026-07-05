// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvTCP.h"

#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    class FilesModelItem;
    class SftpTunnelClient;
    class SftpTunnelServer;
    class WebRTCManager;

    class CommandInterpreter
    {
    public:
        CommandInterpreter(ViewerUI*);
        ~CommandInterpreter();

        void syncFile(
            const std::string& path, const std::string& audioPath,
            const FilesModelItem& item);

    protected:
        void parse(const Message& message);
        void syncMedia(const Message& message);
        void syncUI();
        void handlePeerDisconnected(const std::string& peerId);

        // sFTP additions
        std::shared_ptr<SftpTunnelClient>
        getOrCreateTunnel(WebRTCManager& manager, const std::string& peerId);
        tl::file::Path cachePathFor(const tl::file::Path& remotePath) const;
        void fetchRemoteFile(const std::string& peerId,
                             const tl::file::Path& filePath,
                             const tl::file::Path& audioFilePath,
                             const FilesModelItem& item);

    public:
        void timerEvent();

        static void timerEvent_cb(void* d);
        
        void shutdownSftpTransfers();
        void shutdownTunnels();
        
    private:
        ViewerUI* ui;
        
        
        std::atomic<bool> shuttingDown_{false};
        std::vector<std::thread> activeSftpDownloads_;
        std::mutex downloadThreadsMutex_;
        std::unordered_map<std::string, std::string> fileSourcePeer_;
        std::unordered_map<std::string, std::shared_ptr<SftpTunnelClient> > peerTunnels_;
        
        Poco::UInt16 nextTunnelPort_ = 2222;
        std::mutex tunnelMutex_;
    };

} // namespace mrv
