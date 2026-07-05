// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvSftpClient.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"

#include <tlCore/StringFormat.h>

#include <libssh2.h>
#include <libssh2_sftp.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace
{
    const char* kModule = "sftp";
    
    // Standard cross-platform wrapper to close network sockets
    void closeSocket(int fd)
    {
        if (fd != -1)
        {
#ifdef _WIN32
            closesocket(fd);
#else
            close(fd);
#endif
        }
    }

    // Helper to extract the last session error message string from libssh2
    std::string getLastError(LIBSSH2_SESSION* session)
    {
        char* msg = nullptr;
        libssh2_session_last_error(session, &msg, nullptr, 0);
        return msg ? msg : "Unknown SSH error";
    }

    // RAII wrappers for automatic cleanup on early-return paths.
    struct Libssh2SessionDeleter
    {
        void operator()(LIBSSH2_SESSION* s) const
        {
            if (s)
            {
                libssh2_session_disconnect(s, "Normal Shutdown");
                libssh2_session_free(s);
            }
        }
    };
    using Libssh2SessionPtr = std::unique_ptr<LIBSSH2_SESSION, Libssh2SessionDeleter>;

    struct Libssh2SftpDeleter
    {
        void operator()(LIBSSH2_SFTP* s) const
        {
            if (s)
                libssh2_sftp_shutdown(s);
        }
    };
    using Libssh2SftpPtr = std::unique_ptr<LIBSSH2_SFTP, Libssh2SftpDeleter>;

    struct Libssh2SftpHandleDeleter
    {
        void operator()(LIBSSH2_SFTP_HANDLE* h) const
        {
            if (h)
                libssh2_sftp_close(h);
        }
    };
    using Libssh2SftpHandlePtr = std::unique_ptr<LIBSSH2_SFTP_HANDLE, Libssh2SftpHandleDeleter>;

    // Establishes standard low-level TCP connection
    int connectTcp(const std::string& host, uint16_t port)
    {
        struct addrinfo hints, *res, *p;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        std::string portStr = std::to_string(port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0)
            return -1;

        int sock = -1;
        for (p = res; p != nullptr; p = p->ai_next)
        {
            sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sock == -1)
                continue;

            if (connect(sock, p->ai_addr, p->ai_addrlen) == 0)
                break;

            closeSocket(sock);
            sock = -1;
        }
        freeaddrinfo(res);
        return sock;
    }

    // Verifies remote host key against OpenSSH format known_hosts file (TOFU logic)
    bool verifyKnownHost(LIBSSH2_SESSION* session, const std::string& host, int port)
    {
        LIBSSH2_KNOWNHOSTS* kh = libssh2_knownhost_init(session);
        if (!kh)
        {
            LOG_ERROR(_("SFTP: Failed to initialize known hosts subsystem."));
            return false;
        }

        std::string khPath;
        const char* home = std::getenv("HOME");
        if (!home)
            home = std::getenv("USERPROFILE"); // Windows fallback
        if (home)
            khPath = std::string(home) + "/.ssh/known_hosts";

        if (!khPath.empty())
        {
            // Read existing known hosts if file is present
            libssh2_knownhost_readfile(kh, khPath.c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
        }

        size_t len = 0;
        int type = 0;
        const char* fingerprint = libssh2_session_hostkey(session, &len, &type);
        if (!fingerprint)
        {
            libssh2_knownhost_free(kh);
            LOG_ERROR(_("SFTP: Could not retrieve remote host key."));
            return false;
        }

        // Determine specific key type matching for OpenSSH formats
        int checkType = LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW;
        switch (type)
        {
            case LIBSSH2_HOSTKEY_TYPE_RSA:       checkType |= LIBSSH2_KNOWNHOST_KEY_SSHRSA; break;
            case LIBSSH2_HOSTKEY_TYPE_DSS:       checkType |= LIBSSH2_KNOWNHOST_KEY_SSHDSS; break;
            case LIBSSH2_HOSTKEY_TYPE_ECDSA_256: checkType |= LIBSSH2_KNOWNHOST_KEY_ECDSA_256; break;
            case LIBSSH2_HOSTKEY_TYPE_ECDSA_384: checkType |= LIBSSH2_KNOWNHOST_KEY_ECDSA_384; break;
            case LIBSSH2_HOSTKEY_TYPE_ECDSA_521: checkType |= LIBSSH2_KNOWNHOST_KEY_ECDSA_521; break;
            case LIBSSH2_HOSTKEY_TYPE_ED25519:   checkType |= LIBSSH2_KNOWNHOST_KEY_ED25519; break;
        }

        struct libssh2_knownhost* hostMatch = nullptr;
        int rc = libssh2_knownhost_checkp(kh, host.c_str(), port, fingerprint, len,
                                          LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                          &hostMatch);

        bool success = false;
        switch (rc)
        {
        case LIBSSH2_KNOWNHOST_CHECK_MATCH:
            success = true;
            break;

        case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
            LOG_ERROR(_("SFTP: remote host key has CHANGED since last "
                        "connection. Refusing to connect - this could "
                        "indicate the host was reinstalled, or a "
                        "man-in-the-middle attack."));
            break;

        case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
            // First time seeing this host - trust on first use (TOFU)
            libssh2_knownhost_addc(kh, host.c_str(), nullptr, fingerprint, len,
                                   "mrv2-client", strlen("mrv2-client"), checkType, nullptr);
            if (!khPath.empty())
            {
                if (libssh2_knownhost_writefile(kh, khPath.c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH) != 0)
                {
                    LOG_WARNING(_("SFTP: could not write known_hosts entry; continuing anyway."));
                }
            }
            success = true;
            break;

        case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
        default:
            LOG_ERROR(_("SFTP: error checking known hosts configuration."));
            break;
        }

        libssh2_knownhost_free(kh);
        return success;
    }

    bool authenticate(LIBSSH2_SESSION* session, const std::string& user, const mrv::SftpCredentials& creds)
    {
        std::string privKey = creds.identityFile;
        if (privKey.empty())
        {
            const char* home = std::getenv("HOME");
            if (!home)
                home = std::getenv("USERPROFILE");
            if (home)
            {
                privKey = std::string(home) + "/.ssh/id_ed25519";
                // Fallback attempt on a typical default OpenSSH key location
                if (!mrv::file::exists(privKey))
                    privKey = std::string(home) + "/.ssh/id_rsa";
            }
        }

        // Note: Modern libssh2 versions accept passing nullptr for the public key
        // parameter if it can extract it natively from the private key file.
        int rc = libssh2_userauth_publickey_fromfile(session, user.c_str(), nullptr, privKey.c_str(), nullptr);
        if (rc == 0)
            return true;

        if (!creds.password.empty())
        {
            int rc = libssh2_userauth_password(session, user.c_str(),
                                               creds.password.c_str());
            if (rc == 0)
                return true;
        }

        const std::string msg =
            tl::string::Format(_("SFTP: public-key authentication failed: {0}"))
                .arg(getLastError(session));
        LOG_ERROR(msg);
        return false;
    }
} // namespace

namespace mrv
{
    uint32_t SftpClient::instances = 0;
    
    SftpClient::SftpClient(const std::string& host, uint16_t port, const SftpCredentials& creds)
        : m_host(host)
        , m_port(port)
        , m_creds(creds)
    {
        if (instances == 0)
        {
            if (libssh2_init(0) != 0)
            {
                LOG_ERROR(_("SFTP: Global libssh2 initialization failed."));
                return;
            }
        }
        ++instances;
    }

    SftpClient::~SftpClient()
    {
        --instances;
        if (instances == 0)
        {
            libssh2_exit();
        }
    }
    
    bool SftpClient::downloadFile(
        const std::string& remotePath, 
        const std::string& localPath,
        const std::function<void(uint64_t done, uint64_t total)>& progressCb)
    {
        int sock = connectTcp(m_host, m_port);
        if (sock == -1)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: TCP connection to {0}:{1} failed."))
                    .arg(m_host).arg(m_port);
            LOG_ERROR(msg);
            return false;
        }

        Libssh2SessionPtr session(libssh2_session_init());
        if (!session)
        {
            LOG_ERROR(_("SFTP: could not allocate ssh session."));
            closeSocket(sock);
            return false;
        }

        LIBSSH2_SESSION* raw = session.get();
        if (!raw)
        {
            LOG_ERROR(_("SFTP: could not get raw session."));
            closeSocket(sock);
            return false;
        }

        // Perform the basic SSH handshake protocol
        int rc = libssh2_session_handshake(raw, sock);
        if (rc != 0)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: SSH handshake failed: {0}"))
                    .arg(getLastError(raw));
            LOG_ERROR(msg);
            closeSocket(sock);
            return false;
        }

        if (!verifyKnownHost(raw, m_host, m_port))
        {
            closeSocket(sock);
            return false;
        }

        std::string user = m_creds.user;
        if (user.empty())
        {
            const char* envUser = std::getenv("USER");
            if (!envUser)
                envUser = std::getenv("USERNAME"); // Windows
            if (envUser)
                user = envUser;
            else
                user = "guest";
        }

        if (!authenticate(raw, user, m_creds))
        {
            closeSocket(sock);
            return false;
        }

        Libssh2SftpPtr sftp(libssh2_sftp_init(raw));
        if (!sftp)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not create sftp session: {0}"))
                    .arg(getLastError(raw));
            LOG_ERROR(msg);
            closeSocket(sock);
            return false;
        }

        // Remote stat sizing checks
        LIBSSH2_SFTP_ATTRIBUTES attrs;
        uint64_t remoteSize = 0;
        rc = libssh2_sftp_stat_ex(sftp.get(), remotePath.c_str(), remotePath.length(),
                                  LIBSSH2_SFTP_STAT, &attrs);
        if (rc == 0 && (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE))
        {
            remoteSize = attrs.filesize;
        }
        else
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not stat remote file {0}; proceeding without a known size."))
                    .arg(remotePath);
            LOG_WARNING(msg);
        }

        LIBSSH2_SFTP_HANDLE* remoteFileRaw = libssh2_sftp_open_ex(
            sftp.get(), remotePath.c_str(), remotePath.length(),
            LIBSSH2_FXF_READ, 0, LIBSSH2_SFTP_OPENFILE);

        Libssh2SftpHandlePtr remoteFile(remoteFileRaw);
        if (!remoteFile)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not open remote file {0}: {1}"))
                    .arg(remotePath).arg(getLastError(raw));
            LOG_ERROR(msg);
            closeSocket(sock);
            return false;
        }

        const std::string partPath = localPath + ".part";
        FILE* out = std::fopen(partPath.c_str(), "wb");
        if (!out)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not open local file {0} for writing."))
                    .arg(partPath);
            LOG_ERROR(msg);
            closeSocket(sock);
            return false;
        }

        const size_t kChunkSize = 256 * 1024;
        std::vector<char> buf(kChunkSize);
        uint64_t totalRead = 0;
        bool ok = true;

        while (true)
        {
            // libssh2 returns bytes read, 0 on EOF, or negative value on error
            ssize_t n = libssh2_sftp_read(remoteFile.get(), buf.data(), buf.size());
            if (n == 0)
                break; // EOF
            if (n < 0)
            {
                const std::string msg =
                    tl::string::Format(_("SFTP: read error on {0}: {1}"))
                        .arg(remotePath).arg(getLastError(raw));
                LOG_ERROR(msg);
                ok = false;
                break;
            }

            size_t written = std::fwrite(buf.data(), 1, static_cast<size_t>(n), out);
            if (written != static_cast<size_t>(n))
            {
                const std::string msg =
                    tl::string::Format(_("SFTP: local write error writing {0}."))
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

        // Explicit structural teardown before managing local file manipulation paths
        remoteFile.reset();
        sftp.reset();
        session.reset();
        closeSocket(sock);

        if (!ok)
        {
            std::remove(partPath.c_str());
            return false;
        }

        if (remoteSize != 0 && totalRead != remoteSize)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: transfer of {0} incomplete: got {1} of {2} bytes."))
                    .arg(remotePath).arg(totalRead).arg(remoteSize);
            LOG_ERROR(msg);
            std::remove(partPath.c_str());
            return false;
        }

        if (std::rename(partPath.c_str(), localPath.c_str()) != 0)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: could not rename {0} to {1}."))
                    .arg(partPath).arg(localPath);
            LOG_ERROR(msg);
            std::remove(partPath.c_str());
            return false;
        }

        return true;
    }


    bool SftpClient::downloadFile(
        const tl::file::Path& remotePath, 
        const std::string& localPath,
        const std::function<void(uint64_t done, uint64_t total)>& progressCb)
    {
        auto frames = remotePath.getFrames();
        if (!frames.has_value())
            return downloadFile(remotePath.get(), localPath, progressCb);

        const math::Int64Range range = frames.value();
        const bool listdir = true;
        for (int64_t i = range.getMin(); i <= range.getMax(); ++i)
        {
            std::string remoteFile = remotePath.getFrame(i, listdir);
            bool ok = downloadFile(remoteFile, localPath, progressCb);
            if (!ok) return false;
        }
        return true;
    }
    
} // namespace mrv
