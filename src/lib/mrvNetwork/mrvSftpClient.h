// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <string>
#include <functional>
#include <cstdint>

namespace mrv
{
    struct SftpCredentials
    {
        std::string user;          // defaults to current OS user if empty
        std::string password;      // if not using identityFile
        std::string identityFile;  // e.g. ~/.ssh/id_ed25519; empty = let
                                   // libssh try its default identities +
                                   // running agent
    };

    class SftpClient
    {
    public:
        // Pass connection-specific details once during instantiation
        SftpClient(const std::string& host, uint16_t port, const SftpCredentials& creds);
        ~SftpClient();

        // Delete copy constructor and assignment operator to avoid accidental
        // double-calls to libssh2_exit() via copied destructors.
        SftpClient(const SftpClient&) = delete;
        SftpClient& operator=(const SftpClient&) = delete;

        // Downloads a single remote file over SFTP to a local path, using an
        // atomic-rename staging file (<localPath>.part).
        bool downloadFile(
            const std::string& remotePath, 
            const std::string& localPath,
            const std::function<void(uint64_t done, uint64_t total)>&
            progressCb = nullptr);
        
        // Downloads a single remote file over SFTP to a local path, using an
        // atomic-rename staging file (<localPath>.part).
        bool downloadFile(
            const tl::file::Path& remotePath, 
            const std::string& localPath,
            const std::function<void(uint64_t done, uint64_t total)>&
            progressCb = nullptr);

    private:
        static uint32_t instances;
        std::string m_host;
        uint16_t m_port;
        SftpCredentials m_creds;
    };

} // namespace mrv
