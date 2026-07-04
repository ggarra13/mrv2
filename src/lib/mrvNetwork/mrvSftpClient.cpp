// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvSftpClient.h"

#include "mrvFl/mrvIO.h"
#include "mrvCore/mrvHome.h"

#include <tlCore/StringFormat.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

namespace
{
    const char* kModule = "sftp";

    // RAII wrappers so every early-return path below still cleans up.
    struct SshSessionDeleter
    {
        void operator()(ssh_session s) const
        {
            if (s)
            {
                ssh_disconnect(s);
                ssh_free(s);
            }
        }
    };
    using SshSessionPtr = std::unique_ptr<ssh_session_struct, SshSessionDeleter>;

    struct SftpSessionDeleter
    {
        void operator()(sftp_session s) const
        {
            if (s)
                sftp_free(s);
        }
    };
    using SftpSessionPtr = std::unique_ptr<sftp_session_struct, SftpSessionDeleter>;

    struct SftpFileDeleter
    {
        void operator()(sftp_file f) const
        {
            if (f)
                sftp_close(f);
        }
    };
    using SftpFilePtr = std::unique_ptr<sftp_file_struct, SftpFileDeleter>;

    struct SftpAttributesDeleter
    {
        void operator()(sftp_attributes a) const
        {
            if (a)
                sftp_attributes_free(a);
        }
    };
    using SftpAttributesPtr =
        std::unique_ptr<sftp_attributes_struct, SftpAttributesDeleter>;

    // Verifies the remote host key against the user's known_hosts,
    // adding a first-seen entry automatically (TOFU — trust on first
    // use), same default behavior as the openssh CLI client, but
    // refusing outright on a *changed* key, which indicates either a
    // reinstalled host or a real MITM attempt.
    bool verifyKnownHost(ssh_session session)
    {
        enum ssh_known_hosts_e state = ssh_session_is_known_server(session);

        switch (state)
        {
        case SSH_KNOWN_HOSTS_OK:
            return true;

        case SSH_KNOWN_HOSTS_CHANGED:
            LOG_ERROR(_("SFTP: remote host key has CHANGED since last "
                        "connection. Refusing to connect - this could "
                        "indicate the host was reinstalled, or a "
                        "man-in-the-middle attack."));
            return false;

        case SSH_KNOWN_HOSTS_OTHER:
            LOG_ERROR(_("SFTP: remote offered a host key of a type we "
                        "didn't expect for this host. Refusing to "
                        "connect."));
            return false;

        case SSH_KNOWN_HOSTS_NOT_FOUND:
        case SSH_KNOWN_HOSTS_UNKNOWN:
        {
            // First time seeing this host - record it (TOFU) rather
            // than prompting, since this path runs on a background
            // thread with no UI to prompt through. If you want an
            // explicit user confirmation step instead, this is where
            // to add it (surface the key fingerprint via
            // ssh_get_publickey_hash and route through Fl::awake to
            // ask before calling ssh_session_update_known_hosts).
            int rc = ssh_session_update_known_hosts(session);
            if (rc != SSH_OK)
            {
                LOG_WARNING(_("SFTP: could not write known_hosts entry; "
                              "continuing anyway."));
            }
            return true;
        }

        case SSH_KNOWN_HOSTS_ERROR:
        default:
        {
            const std::string msg =
                tl::string::Format(_("SFTP: error checking known "
                                      "hosts: {0}"))
                    .arg(ssh_get_error(session));
            LOG_ERROR(msg);
            return false;
        }
        }
    }

    bool authenticate(ssh_session session, const mrv::SftpCredentials& creds)
    {
        if (!creds.identityFile.empty())
        {
            ssh_options_set(session, SSH_OPTIONS_IDENTITY,
                             creds.identityFile.c_str());
        }

        // Tries the running agent first, then default identity files
        // (~/.ssh/id_ed25519, id_rsa, etc.) plus whatever was set via
        // SSH_OPTIONS_IDENTITY above. No password prompt involved.
        int rc = ssh_userauth_publickey_auto(session, nullptr, nullptr);
        if (rc == SSH_AUTH_SUCCESS)
            return true;

        const std::string msg =
            tl::string::Format(_("SFTP: public-key authentication "
                                  "failed: {0}"))
                .arg(ssh_get_error(session));
        LOG_ERROR(msg);
        return false;
    }
} // namespace

namespace mrv
{
    bool sftpDownloadFile(
        const std::string& host, uint16_t port,
        const std::string& remotePath, const std::string& localPath,
        const SftpCredentials& creds,
        const std::function<void(uint64_t done, uint64_t total)>& progressCb)
    {
        SshSessionPtr session(ssh_new());
        if (!session)
        {
            LOG_ERROR(_("SFTP: could not allocate ssh session."));
            return false;
        }

        ssh_session raw = session.get();

        std::string user = creds.user;
        if (user.empty())
        {
            const char* envUser = std::getenv("USER");
            if (!envUser)
                envUser = std::getenv("USERNAME"); // Windows
            if (envUser)
                user = envUser;
        }

        ssh_options_set(raw, SSH_OPTIONS_HOST, host.c_str());
        int portInt = static_cast<int>(port);
        ssh_options_set(raw, SSH_OPTIONS_PORT, &portInt);
        if (!user.empty())
            ssh_options_set(raw, SSH_OPTIONS_USER, user.c_str());

        int rc = ssh_connect(raw);
        if (rc != SSH_OK)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: connection to {0}:{1} "
                                      "failed: {2}"))
                    .arg(host).arg(port).arg(ssh_get_error(raw));
            LOG_ERROR(msg);
            return false;
        }

        if (!verifyKnownHost(raw))
            return false;

        if (!authenticate(raw, creds))
            return false;

        SftpSessionPtr sftp(sftp_new(raw));
        if (!sftp)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not create sftp "
                                      "session: {0}"))
                    .arg(ssh_get_error(raw));
            LOG_ERROR(msg);
            return false;
        }

        rc = sftp_init(sftp.get());
        if (rc != SSH_OK)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: sftp_init failed: {0}"))
                    .arg(sftp_get_error(sftp.get()));
            LOG_ERROR(msg);
            return false;
        }

        // Stat first so we can report progress against a known total
        // and detect a missing/unreadable remote file up front rather
        // than failing on the first sftp_read.
        SftpAttributesPtr attrs(sftp_stat(sftp.get(), remotePath.c_str()));
        uint64_t remoteSize = attrs ? attrs->size : 0;
        if (!attrs)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not stat remote file "
                                      "{0}; proceeding without a known "
                                      "size."))
                    .arg(remotePath);
            LOG_WARNING(msg);
        }

        SftpFilePtr remoteFile(sftp_open(sftp.get(), remotePath.c_str(),
                                         O_RDONLY, 0));
        if (!remoteFile)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not open remote file "
                                      "{0}: {1}"))
                    .arg(remotePath).arg(sftp_get_error(sftp.get()));
            LOG_ERROR(msg);
            return false;
        }

        // Stage to <localPath>.part and rename on success, so a
        // failure or cancellation never leaves a partial file at the
        // path callers treat as "this file is ready to open."
        const std::string partPath = localPath + ".part";
        FILE* out = std::fopen(partPath.c_str(), "wb");
        if (!out)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not open local file "
                                      "{0} for writing."))
                    .arg(partPath);
            LOG_ERROR(msg);
            return false;
        }

        const size_t kChunkSize = 256 * 1024;
        std::vector<char> buf(kChunkSize);
        uint64_t totalRead = 0;
        bool ok = true;

        while (true)
        {
            ssize_t n = sftp_read(remoteFile.get(), buf.data(), buf.size());
            if (n == 0)
                break; // EOF
            if (n < 0)
            {
                const std::string msg =
                    tl::string::Format(_("SFTP: read error on {0}: {1}"))
                        .arg(remotePath).arg(sftp_get_error(sftp.get()));
                LOG_ERROR(msg);
                ok = false;
                break;
            }

            size_t written = std::fwrite(buf.data(), 1,
                                          static_cast<size_t>(n), out);
            if (written != static_cast<size_t>(n))
            {
                const std::string msg =
                    tl::string::Format(_("SFTP: local write error "
                                          "writing {0}."))
                        .arg(partPath);
                LOG_ERROR(msg);
                ok = false;
                break;
            }

            totalRead += static_cast<uint64_t>(n);
            if (progressCb)
                progressCb(totalRead, remoteSize);
        }

        std::fclose(out);

        if (!ok)
        {
            std::remove(partPath.c_str());
            return false;
        }

        if (remoteSize != 0 && totalRead != remoteSize)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: transfer of {0} incomplete: "
                                      "got {1} of {2} bytes."))
                    .arg(remotePath).arg(totalRead).arg(remoteSize);
            LOG_ERROR(msg);
            std::remove(partPath.c_str());
            return false;
        }

        if (std::rename(partPath.c_str(), localPath.c_str()) != 0)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not rename {0} to "
                                      "{1}."))
                    .arg(partPath).arg(localPath);
            LOG_ERROR(msg);
            std::remove(partPath.c_str());
            return false;
        }

        return true;
    }

} // namespace mrv
