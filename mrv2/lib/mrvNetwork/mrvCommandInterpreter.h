// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvNetwork/mrvTCP.h"

class ViewerUI;

namespace mrv
{
    class FilesModelItem;

    class CommandInterpreter
    {
    public:
        CommandInterpreter(ViewerUI*);
        ~CommandInterpreter();

    protected:
        void parse(const Message& message);
        void syncMedia(const Message& message);

        void syncFile(
            const std::string& path, const std::string& audioPath,
            const FilesModelItem& item);

    public:
        void timerEvent();

        static void timerEvent_cb(void* d);

    private:
        ViewerUI* ui;
    };

} // namespace mrv
