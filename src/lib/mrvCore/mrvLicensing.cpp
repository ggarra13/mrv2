// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvGlobals.h"

#include "mrvFl/mrvIO.h"

#include "mrvWidgets/mrvVersion.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvOS/mrvI8N.h"

#include "mrvOS/mrvOS.h"

#ifdef MRV2_NETWORK
#    include <Poco/Net/HTTPSClientSession.h>
#    include <Poco/Net/HTTPRequest.h>
#    include <Poco/Net/HTTPResponse.h>
#    include <Poco/Net/NetException.h>
#    include <Poco/StreamCopier.h>
#    include <Poco/Dynamic/Var.h>
#    include <Poco/Exception.h>
#    include <Poco/Net/DNS.h>        // newer Poco versions
#    include <Poco/DateTimeFormatter.h>
#    include <Poco/DateTimeParser.h>
#    include <Poco/Timespan.h>
#    include <Poco/Path.h>
#    include <Poco/Logger.h>
#    include <Poco/ConsoleChannel.h>
#    include <Poco/AutoPtr.h>
#    include <Poco/PatternFormatter.h>
#    include <Poco/FormattingChannel.h>
#endif

#include <nlohmann/json.hpp>

#ifdef TLRENDER_NET
#    include <openssl/evp.h>
#    include <openssl/pem.h>
#    include <openssl/bio.h>
#    include <openssl/buffer.h>
#    include <openssl/err.h>
#    include <openssl/ssl.h>
#    include <openssl/x509_vfy.h>
#endif

#include <tlCore/StringFormat.h>
#include <tlCore/Error.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>

#ifdef _WIN32
#  include <windows.h>
#endif

namespace
{
    const char* kModule = "lic.";

    // --- Load verify key (Base64) ---
    const std::string verify_key_b64 = "V/7Jp4Ngx1g1sSiovQgxl9utH4oylriTMZFNNLHefGU=";
}

namespace
{

#if defined(TLRENDER_SSL) || defined(TLRENDER_NET)
    // -------------------------
    // Base64 decode helper
    // -------------------------
    std::vector<unsigned char> base64_decode(const std::string& in) {
        BIO* bio, * b64;
        int maxLen = in.length() * 3 / 4 + 1;
        std::vector<unsigned char> out(maxLen);

        bio = BIO_new_mem_buf(in.data(), static_cast<int>(in.length()));
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // no newlines
        int len = BIO_read(bio, out.data(), static_cast<int>(in.length()));
        BIO_free_all(bio);

        if (len > 0)
            out.resize(len);
        else
            out.clear();

        return out;
    }

    // -------------------------
    // Base64 encode helper
    // -------------------------
    std::string base64_encode(const unsigned char* data, size_t len) {
        BIO* bio, * b64;
        BUF_MEM* bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // no newlines
        BIO_write(bio, data, static_cast<int>(len));
        BIO_flush(bio);

        BIO_get_mem_ptr(bio, &bufferPtr);
        std::string out(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);
        return out;
    }

    // Convenience overload for std::vector<unsigned char>
    std::string base64_encode(const std::vector<unsigned char>& in) {
        return base64_encode(in.data(), in.size());
    }

    std::string base64_encode(const std::string& in) {
        return base64_encode(reinterpret_cast<const unsigned char*>(in.data()),
                             in.size());
    }

    bool verify_ed25519(const std::string& pubkey_b64,
                        const std::string& message,
                        const std::string& signature_b64) {
        auto pubkey_bytes = base64_decode(pubkey_b64);
        if (pubkey_bytes.size() != 32) {
            LOG_ERROR("Error: Invalid public key length. Expected 32, got "
                      << pubkey_bytes.size() << ".");
            return false;
        }

        auto sig_bytes = base64_decode(signature_b64);
        if (sig_bytes.empty()) {
            LOG_ERROR("Error: Base64 decoding of signature failed.");
            return false;
        }

        EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr,
                                                     pubkey_bytes.data(),
                                                     pubkey_bytes.size());
        if (!pkey) {
            LOG_ERROR("OpenSSL Error: Failed to load Ed25519 public key.");
            ERR_print_errors_fp(stderr);
            return false;
        }

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            LOG_ERROR("OpenSSL Error: Failed to create EVP_MD_CTX.");
            EVP_PKEY_free(pkey);
            return false;
        }

        // 1. Initialize the verification context with the public key.
        if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) != 1) {
            LOG_ERROR("OpenSSL Error: Failed to initialize digest verification.");
            ERR_print_errors_fp(stderr);
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        // 2. Perform the verification in a single step.
        // This is the correct pattern for Ed25519.
        int result = EVP_DigestVerify(ctx,
                                      sig_bytes.data(), sig_bytes.size(),
                                      reinterpret_cast<const unsigned char*>(message.data()), message.size());

        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);

        if (result == 1) {
            return true; // Signature is valid
        } else if (result == 0) {
            return false; // Signature is invalid
        } else {
            LOG_ERROR("OpenSSL Error: An error occurred during verification.");
            ERR_print_errors_fp(stderr);
            return false;
        }
    }
#else
    bool verify_ed25519(const std::string& pubkey_b64,
                        const std::string& message,
                        const std::string& signature_b64)
    {
        return false;
    }
#endif

    /**
     * Main entry function to turn on / off the options based on license
     * Plan
     *
     * @param plan valid plan name
     */
    void activatePlan(const std::string& plan)
    {
        if (plan == "Pro" || plan == "Pro+")
        {
            mrv::app::soporta_annotations = true;
            mrv::app::soporta_editing = true;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = true;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = true;
        }
        else if (plan == "Edit")
        {
            mrv::app::soporta_annotations = true;
            mrv::app::soporta_editing = true;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = true;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = false;
        }
        else if (plan == "Standard")
        {
            mrv::app::soporta_annotations = true;
            mrv::app::soporta_editing = false;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = true;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = false;
        }
        else if (plan == "Solo")
        {
            mrv::app::soporta_annotations = true;
            mrv::app::soporta_editing = false;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = false;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = false;
        }
        else if (plan == "Demo")
        {
            mrv::app::soporta_annotations = false;
            mrv::app::soporta_editing = false;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = false;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = false;
        }
        else
        {
            // Unknown license plan
            mrv::app::soporta_annotations = false;
            mrv::app::soporta_editing = false;
            mrv::app::soporta_layers = true;
            mrv::app::soporta_python = false;
            mrv::app::soporta_saving = true;
            mrv::app::soporta_voice = false;

            const std::string msg =
                tl::string::Format(_("Unknown licese plan '{0}'")).arg(plan);
            LOG_ERROR(msg);
        }
        std::string msg = tl::string::Format(_("License plan '{0}'")).arg(plan);
        LOG_STATUS(msg);
        msg = tl::string::Format(_("Supports annotations '{0}'")).arg(mrv::app::soporta_annotations);
        LOG_INFO(msg);
        msg = tl::string::Format(_("Supports editing '{0}'")).arg(mrv::app::soporta_editing);
        LOG_INFO(msg);
        msg = tl::string::Format(_("Supports layers '{0}'")).arg(mrv::app::soporta_layers);
        LOG_INFO(msg);
        msg = tl::string::Format(_("Supports python '{0}'")).arg(mrv::app::soporta_python);
        LOG_INFO(msg);
        msg = tl::string::Format(_("Supports voice annotations '{0}'")).arg(mrv::app::soporta_voice);
        LOG_INFO(msg);
    }

    /**
     * Function used to strip spaces and new lines from a string from the
     * output of a command or license return string.
     *
     * @param output string to strip the spaces and newlines
     *
     * @return stripped string
     */
    inline std::string stripOutput(const std::string& output)
    {
        std::string out = output;
        out.erase(remove(out.begin(), out.end(), '\n'), out.end());
        out.erase(remove(out.begin(), out.end(), '\r'), out.end());
        out.erase(remove(out.begin(), out.end(), ' '), out.end());
        return out;
    }
}

namespace mrv
{
    TLRENDER_ENUM_IMPL(LicenseType, _("Demo"), _("Node-Locked"), _("Floating"));
    TLRENDER_ENUM_SERIALIZE_IMPL(LicenseType);

    std::vector<std::string> get_machine_ids() {
        std::vector<std::string> out;
        std::string output;
        std::string errors;

#if defined(_WIN32)
        //
        // Due to legacy issues, on AMD64 we relied on wmic for the license.
        //        std::string errors;
        try
        {
            mrv::os::exec_command("powershell \"Get-CimInstance -ClassName Win32_ComputerSystemProduct | Select-Object -ExpandProperty UUID\"", output, errors);
            // mrv::os::exec_command("wmic csproduct get uuid", output,
            //                       errors);
            out.push_back(stripOutput(output));
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(e.what());
            LOG_ERROR(errors);
        }
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "SOFTWARE\\Microsoft\\Cryptography",
                          0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            char value[256];
            DWORD value_length = sizeof(value);
            if (RegGetValueA(hKey, nullptr, "MachineGuid",
                             RRF_RT_REG_SZ, nullptr,
                             &value, &value_length) == ERROR_SUCCESS)
            {
                out.push_back(stripOutput(value));
            }
            RegCloseKey(hKey);
        }

#elif defined(__APPLE__)
        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                          "ioreg -rd1 -c IOPlatformExpertDevice | grep IOPlatformUUID | cut -d '\"' -f4", "r"), pclose);
        if (pipe) {
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                output += buffer.data();
            }
            out.push_back(stripOutput(output));
        }
#else
        std::ifstream f("/etc/machine-id");
        std::getline(f, output);
        out.push_back(stripOutput(output));
#endif
        return out;
    }

    /**
     * Common function to return:
     *
     * @param server Server to connect for the license
     * @param port   Port for the request (443 for HTTPS POST request)
     * @param machine_ids List of valid machine_ids for node-locked licenses.
     *                    We use a vector of machine_ids since Windows
     *                    deprecated the wmic command we were originally using
     *                    for the license
     * @param master_key  Master Key for floating licenses.
     *                    Found through environment variable MRV2_LICENSEPATH.
     */
    void get_network_configuration(std::string& server, int& port,
                                   std::vector<std::string>& machine_ids,
                                   std::string& master_key)
    {
        server = os::sgetenv("MRV2_LICENSE_SERVER");
        if (server.empty())
        {
            //server = "srv1037957.hstgr.cloud";
            server = "filmaura.cloud";
        }

        port = 443;

        machine_ids = get_machine_ids();
        master_key = ""; // Only used on floating licenses

        const std::string path = licensepath();
        const std::string license_file = path + "/mrv2_licenses.lic";

        if (file::isReadable(license_file))
        {
            std::string msg = string::Format(_("Found floating license at '{0}'"))
                              .arg(license_file);
            LOG_INFO(msg);

            // Open the file for reading
            std::ifstream file_stream(license_file);

            // Check if the file was successfully opened
            if (!file_stream.is_open())
            {
                LOG_ERROR("Error: Failed to open file: " << license_file);
                return;
            }

            // Slurp file.
            std::stringstream buffer;
            buffer << file_stream.rdbuf();
            master_key = buffer.str();

            // Fix the string to be valid JSON by replacing single quotes
            std::replace(master_key.begin(), master_key.end(), '\'', '\"');
        }
        else
        {
            if (!path.empty())
            {
                std::string msg = string::Format(_("Did not find floating license at '{0}'"))
                                  .arg(path);
                LOG_STATUS(msg);
            }
        }
    }

    /**
     * Check a string for expiration.  Date is expected to be in:
     * YEAR-MM-DD format, with unused time and timezone.
     *
     * @param expires_at License expiration date.
     *
     * @return License Status (kValid or kExpired).
     */
    License has_license_expired(const std::string& expires_at)
    {
        std::tm tm = {};
        std::istringstream ss(expires_at);

        if (!(ss >> std::get_time(&tm, "%Y-%m-%d"))) {
            LOG_ERROR(_("Invalid expiration date"));
            return License::kExpired;
        }

        // expire at END of day
        tm.tm_hour = 23;
        tm.tm_min  = 59;
        tm.tm_sec  = 59;

        std::time_t exp_time = std::mktime(&tm);

        std::time_t now = std::time(nullptr);
        if (now > exp_time) {
            LOG_ERROR(_("License expired"));
            return License::kExpired;
        }

        return License::kValid;
    }

    /**
     * HTTPS post request
     *
     * @param serverHost host for the post request
     * @param serverPort port for the post request (usually 443 = https)
     * @param entryPoint entry point on server for the post request.
     * @param requestBody JSON string for the request body.
     *
     * Uses mrv2's built-in certificate by default which is updated on each
     * build.  As a backup, if it is missing, use the OS's default one.
     *
     * @return a nlohmann::json message.
     */
    nlohmann::json post_request(const std::string serverHost,
                                const int serverPort,
                                const std::string& entryPoint,
                                const std::string& requestBody)
    {
#ifdef MRV2_NETWORK
        using namespace Poco::Net;
        using namespace Poco;

        try
        {
            std::string caLocation = mrv::rootpath() + "/certs/cacert.pem";

            bool useDefault = false;
            if (!file::isReadable(caLocation))
            {
                /* xgettext:c++-format */
                const std::string msg =
                    string::Format(_("{0} is not readable. Using system default.")).
                    arg(caLocation);
                LOG_STATUS(msg);
#ifdef __linux___
                caLocation = "/etc/ssl/certs/ca-certificates.crt";
#elif defined(__APPLE__)
                caLocation = "/usr/local/etc/openssl@3/cert.pem";
#else
                caLocation = "";
#endif
                useDefault = true;
            }


            Context::Ptr context = new Context(
                Context::CLIENT_USE,
                "",    // privateKeyFile
                "",    // certificateFile
                caLocation,    // caLocation ("" = system default, if available)
                Context::VERIFY_STRICT,
                9,     // verificationDepth
                useDefault, // load default CA location
                "ALL"
                );

            HTTPSClientSession session(serverHost, serverPort, context);
            // Shorter connect timeout (e.g. 2 seconds instead of ~20s default)
            session.setConnectTimeout(Poco::Timespan(2, 0));

            // General operation timeout (read/write)
            session.setTimeout(Poco::Timespan(10, 0));

            HTTPRequest request(HTTPRequest::HTTP_POST,
                                           entryPoint,
                                           HTTPMessage::HTTP_1_1);
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());

            std::ostream& os = session.sendRequest(request);
            os << requestBody;

            HTTPResponse response;
            std::istream& rs = session.receiveResponse(response);

            std::ostringstream oss;
            Poco::StreamCopier::copyStream(rs, oss);
            std::string respStr = oss.str();

            if (response.getStatus() == 502)
            {
                LOG_ERROR("Server is down or your internet connection is failing.");
                return nlohmann::json();
            }

            nlohmann::json json_data = nlohmann::json::parse(respStr);

            if (response.getStatus() != HTTPResponse::HTTP_OK)
            {
                // Parse JSON error message
                if (json_data.contains("detail")) {
                    LOG_ERROR(json_data["detail"]);
                } else {
                    LOG_ERROR(respStr);
                }
                return "";
            }

            return json_data;
        }
        catch (const Exception& ex) {
            // Most DNS blocks throw one of these:
            // - HostNotFoundException
            // - DNSException
            // - ConnectionRefusedException (some blockers return 0.0.0.0)
            if (
                dynamic_cast<const ConnectionRefusedException*>(&ex) ||
                dynamic_cast<const HostNotFoundException*>(&ex) ||
                dynamic_cast<const DNSException*>(&ex) ||
                std::string(ex.displayText()).find("Name or service not known") != std::string::npos ||
                std::string(ex.displayText()).find("NXDOMAIN") != std::string::npos)
            {
                std::string msg = "DNS resolution failed – very likely blocked by Pi-hole/AdGuard/hagezi Light";
                LOG_ERROR(msg);
            }

            LOG_ERROR("Request failed: " << ex.displayText());
        }
        catch (const std::exception& ex) {
            LOG_ERROR(ex.what());
        }
#endif

        return nlohmann::json();
    }


    /**
     * Send a hearbeat to license server to verify floating license is still
     * active and renew it.
     *
     * @return true if valid, false if not.
     */
    bool send_heartbeat()
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);

        if (master_key.empty())
            return false;

        // --- Build JSON request ---

        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id and session_id
        request_body_json["machine_id"] = machine_ids[0];
        request_body_json["session_id"] = app::session_id;

        // Parse the corrected master key string and add it as a nested object
        request_body_json["master_key"] = nlohmann::json::parse(master_key);

        // 4. Dump the final, complete object into a string
        // The library handles all formatting and escaping correctly.
        const std::string requestBody = request_body_json.dump();

        // --- HTTP POST to /request_license ---
        nlohmann::json json_data = post_request(serverHost, serverPort,
                                                "/heartbeat", requestBody);
        if (json_data.is_null() || !json_data.contains("status"))
            return false;

        if (json_data["status"] == "renewed")
        {
            LOG_INFO("Renewed license");
            return true;
        }

        return false;
    }

    /**
     * Validate a floating license
     *
     * @param expiration_date Returned expiration date as a string
     *
     * @return License State (kValid, kInvalid, kExpired)
     */
    License validate_floating(std::string& expiration_date)
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);

        if (master_key.empty())
            return License::kInvalid;


        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id
        request_body_json["machine_id"] = machine_ids[0];
        request_body_json["session_id"] = "";

        // Parse the corrected master key string and add it as a nested object
        request_body_json["master_key"] = nlohmann::json::parse(master_key);

        // 4. Dump the final, complete object into a string
        // The library handles all formatting and escaping correctly.
        const std::string requestBody = request_body_json.dump();

        // --- HTTP POST to /checkout_license ---
        nlohmann::ordered_json json_data = post_request(serverHost, serverPort,
                                                        "/checkout_license",
                                                        requestBody);
        if (json_data.is_null() || !json_data.contains("signature"))
        {
            // The error message was already logged inside post_request.
            // We just need to stop here.
            return License::kInvalid;
        }

        // Extract the 'signature' string
        const std::string signature = json_data.at("signature").get<std::string>();

        // Get the 'payload' object with ordered keys
        const nlohmann::ordered_json& payload_json = json_data.at("payload");
        const std::string expires_at = payload_json.at("expires_at").get<std::string>();
        const std::string plan = payload_json.at("plan").get<std::string>();
        const int active_seats = payload_json.at("active_seats").get<int>();
        const int license_limit = payload_json.at("license_limit").get<int>();
        app::session_id = payload_json.at("session_id").get<std::string>();

        std::string msg = string::Format(_("{0} Active Licenses from {1}.")).
                          arg(active_seats).
                          arg(license_limit);
        LOG_STATUS(msg);

        // -------------------------
        // Verify license
        // -------------------------
        const std::string verify_json = payload_json.dump(-1);
        bool valid = verify_ed25519(verify_key_b64, verify_json, signature);

        if (!valid)
        {
            LOG_ERROR("❌ Invalid signature");
            return License::kInvalid;
        }

        expiration_date = expires_at;

        // Activate the options based on Plan.
        activatePlan(plan);

        return License::kValid;
    }

    std::string request_webrtc_ticket_floating()
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);
        if (master_key.empty())
            return "";

        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id
        request_body_json["machine_id"] = machine_ids[0];
        request_body_json["session_id"] = "";

        // Parse the corrected master key string and add it as a nested object
        request_body_json["master_key"] = nlohmann::json::parse(master_key);

        // 4. Dump the final, complete object into a string
        // The library handles all formatting and escaping correctly.
        const std::string requestBody = request_body_json.dump();

        // --- HTTP POST to /checkout_license ---
        nlohmann::ordered_json json_data = post_request(
            serverHost, serverPort,
            "/webrtc_ticket_floating",
            requestBody);
        if (json_data.is_null() || !json_data.contains("signature"))
        {
            // The error message was already logged inside post_request.
            // We just need to stop here.
            return "";
        }

        // -------------------------
        // Prepare the token for the WebSocket URL
        // -------------------------
        // We dump the ENTIRE json (payload + signature) to a compact string
        std::string raw_token_json = json_data.dump(-1);

        // Base64 encode it so it safely passes through the URL query parameter
        // NOTE: Replace `base64_encode` with whatever base64 encoding utility
        // you currently use in your C++ codebase.
        std::string base64_token = base64_encode(raw_token_json);

        return base64_token;
    }

    std::string request_webrtc_ticket_node_locked()
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);

        std::string valid_machine_id;
        nlohmann::json valid_json_data;

        // --- Build JSON request ---
        for (const auto& id : machine_ids)
        {
            const std::string requestVersion = mrv::version();  // legacy
            const std::string requestBody = "{\"machine_id\":\"" +
                                            id + "\",\"plan\":\""
                                            + requestVersion + "\"}";

            // --- HTTP POST to /webrtc_ticket ---
            nlohmann::json json_data = post_request(serverHost, serverPort,
                                                    "/webrtc_ticket",
                                                    requestBody);

            if (json_data.is_null() ||
                !json_data.contains("signature") ||
                !json_data.contains("payload"))
                continue;

            valid_machine_id = id;
            valid_json_data = json_data;
            break; // Successfully got a ticket
        }

        if (valid_machine_id.empty())
        {
            LOG_ERROR("Could not obtain a WebRTC ticket from the server.");
            return "";
        }

        // --- Parse JSON response with nlohmann::json ---
        std::string signature = valid_json_data.at("signature").get<std::string>();
        const nlohmann::ordered_json& payload_json = valid_json_data.at("payload");

        if (!payload_json.contains("expires_at") ||
            !payload_json.contains("machine_id") ||
            !payload_json.contains("purpose"))
        {
            LOG_ERROR("Malformed ticket payload.");
            return "";
        }

        const std::string expires_at = payload_json.at("expires_at").get<std::string>();
        const std::string payload_machine_id = payload_json.at("machine_id").get<std::string>();
        const std::string purpose = payload_json.at("purpose").get<std::string>();

        // -------------------------
        // Verify server signature locally (Prevents spoofing)
        // -------------------------
        const std::string verify_json = payload_json.dump(-1);
        bool valid = verify_ed25519(verify_key_b64, verify_json, signature);

        if (!valid)
        {
            LOG_ERROR("❌ Invalid signature on WebRTC ticket");
            return "";
        }

        // Double-check the payload data
        if (payload_machine_id != valid_machine_id || purpose != "webrtc_sync")
        {
            LOG_ERROR("WebRTC ticket mismatch (machine_id or purpose invalid).");
            return "";
        }

        // -------------------------
        // Prepare the token for the WebSocket URL
        // -------------------------
        // We dump the ENTIRE json (payload + signature) to a compact string
        std::string raw_token_json = valid_json_data.dump(-1);

        // Base64 encode it so it safely passes through the URL query parameter
        // NOTE: Replace `base64_encode` with whatever base64 encoding utility
        // you currently use in your C++ codebase.
        std::string base64_token = base64_encode(raw_token_json);

        return base64_token;
    }

    std::string request_webrtc_ticket()
    {

        if (app::license_type == LicenseType::kFloating)
            return request_webrtc_ticket_floating();
        else
            return request_webrtc_ticket_node_locked();
    }

    License validate_node_locked(std::string& expiration_date)
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);

        if (!master_key.empty())
            return License::kInvalid;

        // --- Build JSON request ---
        std::string machine_id;
        nlohmann::json json_data;
        for (const auto& id : machine_ids)
        {
            const std::string requestVersion = mrv::version();  // unused - legacy
            const std::string requestBody = "{\"machine_id\":\"" +
                                            id + "\",\"plan\":\""
                                            + requestVersion + "\"}";

            // --- HTTP POST to /node_locked_license ---
            json_data = post_request(serverHost, serverPort,
                                     "/node_locked_license",
                                     requestBody);
            if (json_data.is_null() ||
                !json_data.contains("signature") ||
                !json_data.contains("payload"))
                continue;

            machine_id = id;
            break;
        }

        if (machine_id.empty())
        {
            return License::kInvalid;
        }

        // --- Parse JSON response with nlohmann::json ---

        // Extract the 'signature' string
        std::string signature = json_data.at("signature").get<std::string>();

        // Get the 'payload' object with ordered keys
        const nlohmann::ordered_json& payload_json = json_data.at("payload");

        if (!payload_json.contains("expires_at") ||
            !payload_json.contains("machine_id") ||
            !payload_json.contains("plan"))
            return License::kInvalid;

        const std::string expires_at = payload_json.at("expires_at").get<std::string>();
        const std::string payload_machine_id = payload_json.at("machine_id").get<std::string>();
        const std::string plan = payload_json.at("plan").get<std::string>();


        // -------------------------
        // Verify license
        // -------------------------
        const std::string verify_json = payload_json.dump(-1);
        bool valid = verify_ed25519(verify_key_b64, verify_json, signature);

        if (!valid)
        {
            LOG_ERROR("❌ Invalid signature");
            return License::kInvalid;
        }

        // Check machine_id
        if (payload_machine_id != machine_id) {
            LOG_ERROR("machine_id does not match");
            return License::kInvalid;
        }

        expiration_date = expires_at;

        // Check expiration
        if (has_license_expired(expires_at) == License::kExpired)
        {
            return License::kExpired;
        }

        // Activate the options based on Plan.
        activatePlan(plan);

        return License::kValid;
    }

    bool release_license()
    {
        if (app::license_type != LicenseType::kFloating)
            return true;

        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::vector<std::string> machine_ids;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_ids,
                                  master_key);

        if (master_key.empty())
            return false;

        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id
        request_body_json["machine_id"] = machine_ids[0];
        request_body_json["session_id"] = app::session_id;

        // Parse the corrected master key string and add it as a nested object
        request_body_json["master_key"] = nlohmann::json::parse(master_key);

        // 4. Dump the final, complete object into a string
        // The library handles all formatting and escaping correctly.
        const std::string requestBody = request_body_json.dump();

        // --- HTTP POST to /checkout_license ---
        nlohmann::ordered_json json_data = post_request(serverHost, serverPort,
                                                        "/release_license",
                                                        requestBody);
        if (json_data.is_null() || !json_data.contains("status"))
        {
            LOG_ERROR(_("Could not release floating license"));
            return false;
        }

        if (json_data["status"] == "released")
        {
            LOG_STATUS(_("Released Floating License"));
            return true;
        }

        return false;
    }

    /**
     * Main License Validation routine.
     *
     * @param expiration_date returned reference to a string with the
     *                        expiration date.
     *
     * @return License State (kInvalid, kValid, kExpired)
     *         app::license_type is changed to the LicenseType
     *         (kNodeLocked or kFloating)
     */
    License validate_license(std::string& expiration_date)
    {
        License out = License::kInvalid;

        if (app::license_type == LicenseType::kDemo)
        {
            out = validate_node_locked(expiration_date);
            if (out == License::kValid || out == License::kExpired)
            {
                app::license_type = LicenseType::kNodeLocked;
            }
        }

        if (app::license_type != LicenseType::kNodeLocked)
        {
            if (app::license_type == LicenseType::kFloating)
            {
                if (send_heartbeat())
                    return License::kValid;
                else
                    return License::kInvalid;
            }
            else
            {
                out = validate_floating(expiration_date);
                if (out == License::kValid)
                {
                    app::license_type = LicenseType::kFloating;
                }
            }
        }

        if (out == License::kValid)
        {
            app::demo_mode = false;
        }
        else
        {
            app::demo_mode = true;
        }

        if (app::license_type != LicenseType::kDemo)
        {
            /* xgettext:c++-format */
            std::string msg =
                string::Format(_("Your {0} license will expire on {1}."))
                .arg(app::license_type)
                .arg(expiration_date);
            LOG_STATUS(msg);
        }

        return out;
    }

    /**
     * Validate a floating license after a period.
     *
     * @return License State (kInvalid, kValid, kExpired)
     */
    License license_beat()
    {
        if (app::force_demo)
        {
            app::demo_mode = false;
            return License::kInvalid;
        }

        // On license beat, we don't check the expiration date.
        std::string expiration;
        License ok = validate_license(expiration);
        return ok;
    }
}
