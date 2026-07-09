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

        void syncMedia(const std::string& peerId, const Message& message);
        void syncUI(const std::string& peerId);

        void handlePeerDisconnected(const std::string& peerId);

        tl::file::Path cachePathFor(const tl::file::Path& remotePath) const;
        void fetchRemoteFile(const std::string& peerId,
                             const tl::file::Path& filePath,
                             const tl::file::Path& audioFilePath,
                             const FilesModelItem& item);

    public:
        void timerEvent();
        static void timerEvent_cb(void* d);

    private:
        ViewerUI* ui;
    };

} // namespace mrv
