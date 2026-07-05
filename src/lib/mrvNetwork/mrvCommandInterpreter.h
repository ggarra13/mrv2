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
        std::string cachePathFor(const std::string& remotePath) const;
        void fetchRemoteFile(const std::string& peerId,
                             const tl::file::Path& filePath,
                             const std::string& audioFilePath,
                             const FilesModelItem& item);

    public:
        void timerEvent();

        static void timerEvent_cb(void* d);

    private:
        ViewerUI* ui;
        std::unordered_map<std::string, std::string> fileSourcePeer_;
        std::unordered_map<std::string, std::shared_ptr<SftpTunnelClient> > peerTunnels_;
        Poco::UInt16 nextTunnelPort_ = 2222;
        std::mutex tunnelMutex_;
    };

} // namespace mrv
