// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <functional>
#include <cstdint>

namespace mrv
{
    struct SftpCredentials
    {
        std::string user;          // defaults to current OS user if empty
        std::string identityFile;  // e.g. ~/.ssh/id_ed25519; empty = let
                                    // libssh try its default identities +
                                    // running agent
    };

    // Downloads a single remote file over SFTP to a local path, using an
    // atomic-rename staging file (<localPath>.part) so a crash or
    // cancelled transfer never leaves a corrupt file at localPath.
    //
    // host/port point at wherever an sshd is actually reachable from —
    // in the mesh-tunnel case that's 127.0.0.1:<tunnel port>, but this
    // function has no knowledge of tunneling; it just speaks SFTP to
    // whatever endpoint it's given.
    //
    // progressCb is called with bytes-downloaded-so-far and total bytes
    // (total may be 0 if the remote server didn't report a size).
    // It may be called from this function's calling thread — callers
    // that touch UI state from it must marshal to the UI thread
    // themselves (e.g. via Fl::awake).
    //
    // Returns true on success. On failure, check the log for the
    // specific libssh/sftp error that was recorded.
    bool sftpDownloadFile(
        const std::string& host, uint16_t port,
        const std::string& remotePath, const std::string& localPath,
        const SftpCredentials& creds,
        const std::function<void(uint64_t done, uint64_t total)>& progressCb);

} // namespace mrv
