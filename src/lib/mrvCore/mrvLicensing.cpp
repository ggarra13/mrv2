#include "mrvApp/mrvApp.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvLicensing.h"
#include "mrvCore/mrvOS.h"

#ifdef MRV2_NETWORK
#    include <Poco/Net/HTTPSClientSession.h>
#    include <Poco/Net/HTTPRequest.h>
#    include <Poco/Net/HTTPResponse.h>
#    include <Poco/StreamCopier.h>
#    include <Poco/Dynamic/Var.h>
#    include <Poco/Exception.h>
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

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>


namespace
{
    const char* kModule = "lic.";

    // --- Load verify key (Base64) ---
    const std::string verify_key_b64 = "V/7Jp4Ngx1g1sSiovQgxl9utH4oylriTMZFNNLHefGU=";
}

namespace
{

#ifdef TLRENDER_NET
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
    
    bool verify_ed25519(const std::string& pubkey_b64,
                        const std::string& message,
                        const std::string& signature_b64) {
        auto pubkey_bytes = base64_decode(pubkey_b64);
        if (pubkey_bytes.size() != 32) {
            std::cerr << "Error: Invalid public key length. Expected 32, got " << pubkey_bytes.size() << ".\n";
            return false;
        }

        auto sig_bytes = base64_decode(signature_b64);
        if (sig_bytes.empty()) {
            std::cerr << "Error: Base64 decoding of signature failed.\n";
            return false;
        }

        EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr,
                                                     pubkey_bytes.data(),
                                                     pubkey_bytes.size());
        if (!pkey) {
            std::cerr << "OpenSSL Error: Failed to load Ed25519 public key.\n";
            ERR_print_errors_fp(stderr);
            return false;
        }

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            std::cerr << "OpenSSL Error: Failed to create EVP_MD_CTX.\n";
            EVP_PKEY_free(pkey);
            return false;
        }

        // 1. Initialize the verification context with the public key.
        if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) != 1) {
            std::cerr << "OpenSSL Error: Failed to initialize digest verification.\n";
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
            std::cerr << "OpenSSL Error: An error occurred during verification.\n";
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
    
}

namespace mrv
{
    TLRENDER_ENUM_IMPL(LicenseType, _("Demo"), _("Node-Locked"), _("Floating"));
    TLRENDER_ENUM_SERIALIZE_IMPL(LicenseType);
    
    std::string get_machine_id() {
        std::string out;
        
#if defined(_WIN32)
#  if defined(_M_X64) || defined(_M_AMD64)
        std::string errors;
        mrv::os::exec_command("wmic csproduct get uuid", out, errors);
        size_t pos = out.find("\r\n");
        if (pos != std::string::npos)
        {
            out = out.substr(pos + 2);
        }
#  else
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
                out = value;
            }
            RegCloseKey(hKey);
        }
#  endif
#elif defined(__APPLE__)
        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                          "ioreg -rd1 -c IOPlatformExpertDevice | grep IOPlatformUUID | cut -d '\"' -f4", "r"), pclose);
        if (pipe) {
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                out += buffer.data();
            }
        }
#else
        std::ifstream f("/etc/machine-id");
        std::getline(f, out);
#endif
        out.erase(remove(out.begin(), out.end(), '\n'), out.end());
        out.erase(remove(out.begin(), out.end(), '\r'), out.end());
        out.erase(remove(out.begin(), out.end(), ' '), out.end());
        return out;
    }

    void get_network_configuration(std::string& server, int& port,
                                   std::string& machine_id,
                                   std::string& master_key)
    {
        server = "srv1037957.hstgr.cloud";
        port = 443;
        machine_id = get_machine_id();
        master_key = ""; // Only used on floating licenses

        std::string license_file = studiopath() + "/mrv2_licenses.lic";
        if (!file::isReadable(license_file))
        {
            license_file = prefspath() + "/mrv2_licenses.lic";
        }
        
        if (file::isReadable(license_file))
        {
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
    }
    
    License has_license_expired(const std::string& expires_at)
    {
        std::tm tm = {};
        std::istringstream ss(expires_at);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        std::time_t exp_time = std::mktime(&tm);

        std::time_t now = std::time(nullptr);
        if (now > exp_time) {
            LOG_ERROR(_("License expired"));
            return License::kExpired;
        }
        
        return License::kValid;
    }
    
    
    nlohmann::json post_request(const std::string serverHost,
                                const int serverPort,
                                const std::string& entryPoint,
                                const std::string& requestBody)
    {
#ifdef MRV2_NETWORK
        try
        {
            
            std::string caLocation = mrv::rootpath() + "/certs/cacert.pem";

            bool useDefault = false;
            if (!file::isReadable(caLocation))
            {
                std::string msg =
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

            
            Poco::Net::Context::Ptr context = new Poco::Net::Context(
                Poco::Net::Context::CLIENT_USE,
                "",    // privateKeyFile
                "",    // certificateFile
                caLocation,    // caLocation ("" = system default, if available)
                Poco::Net::Context::VERIFY_STRICT,
                9,     // verificationDepth
                useDefault, // load default CA location
                "ALL"
                );
            
            Poco::Net::HTTPSClientSession session(serverHost, serverPort, context);
            // Shorter connect timeout (e.g. 2 seconds instead of ~20s default)
            session.setConnectTimeout(Poco::Timespan(2, 0));  

            // General operation timeout (read/write)
            session.setTimeout(Poco::Timespan(10, 0));
            
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                           entryPoint,
                                           Poco::Net::HTTPMessage::HTTP_1_1);
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());

            std::ostream& os = session.sendRequest(request);
            os << requestBody;

            Poco::Net::HTTPResponse response;
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
            
            if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
            {
                // Parse JSON error message
                if (json_data.contains("detail")) {
                    LOG_ERROR(json_data["detail"]);
                } else {
                    LOG_ERROR(respStr);
                }
            }
        
            return json_data;
        }
        catch (const Poco::Exception& ex) {
            LOG_ERROR("Request failed: " << ex.displayText());
        }
        catch (const std::exception& ex) {
            LOG_ERROR(ex.what());
        }
#endif
        
        return nlohmann::json();
    }

    
    bool send_heartbeat()
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::string machine_id;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_id,
                                  master_key);

        if (master_key.empty())
            return false;
            
        // --- Build JSON request ---
        
        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id
        request_body_json["machine_id"] = machine_id;

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
            return true;
        }
        
        return false;
    }

    License validate_floating(std::string& expiration_date)
    {
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::string machine_id;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_id,
                                  master_key);
        
        if (master_key.empty())
            return License::kInvalid;

        // Build the request object programmatically.
        nlohmann::json request_body_json;

        // Add the machine_id
        request_body_json["machine_id"] = machine_id;

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
        return License::kValid;
    }
    
    License validate_node_locked(std::string& expiration_date)
    {        
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::string machine_id;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_id,
                                  master_key);
            
        // --- Build JSON request ---
        const std::string plan = "Pro";
        const std::string requestBody = "{\"machine_id\":\"" +
                                        machine_id + "\",\"plan\":\""
                                        + plan + "\"}";

        // --- HTTP POST to /node_locked_license ---
        nlohmann::json json_data = post_request(serverHost, serverPort,
                                                "/node_locked_license",
                                                requestBody);
        if (json_data.is_null() ||
            !json_data.contains("signature") ||
            !json_data.contains("payload"))
            return License::kInvalid;

        // --- Parse JSON response with nlohmann::json ---
            
        // Extract the 'signature' string
        std::string signature = json_data.at("signature").get<std::string>();
            
        // Get the 'payload' object with ordered keys
        const nlohmann::ordered_json& payload_json = json_data.at("payload");

        if (!payload_json.contains("expires_at") ||
            !payload_json.contains("machine_id"))
            return License::kInvalid;
        
        const std::string expires_at = payload_json.at("expires_at").get<std::string>();
        const std::string payload_machine_id = payload_json.at("machine_id").get<std::string>();
        
            
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

        return License::kValid;
    }

    bool release_license()
    {
        if (App::license_type != LicenseType::kFloating)
            return true;
        
        // --- Configuration ---
        std::string serverHost;
        int serverPort;
        std::string machine_id;
        std::string master_key;
        get_network_configuration(serverHost, serverPort, machine_id,
                                  master_key);
        
        if (master_key.empty())
            return false;
        
        // Build the request object programmatically.
        nlohmann::json request_body_json;
        
        // Add the machine_id
        request_body_json["machine_id"] = machine_id;

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
            return false;
        
        if (json_data["status"] == "released")
        {
            LOG_STATUS(_("Released Floating License"));
            return true;
        }
        
        return false;
    }
        
    License validate_license(std::string& expiration_date)
    {
        License out = License::kInvalid;
        
        if (App::license_type == LicenseType::kDemo)
        {
            out = validate_node_locked(expiration_date);
            if (out == License::kValid || out == License::kExpired)
            {
                App::license_type = LicenseType::kNodeLocked;
            }
        }

        if (App::license_type != LicenseType::kNodeLocked)
        {
            if (App::license_type == LicenseType::kFloating)
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
                    App::license_type = LicenseType::kFloating;
                }
            }
        }

        if (out == License::kValid)
        {
            App::demo_mode = false;
        }
        else
        {
            App::demo_mode = true;
        }

        if (App::license_type != LicenseType::kDemo)
        {
            std::string msg =
                string::Format(_("Your {0} license will expire on {1}."))
                .arg(App::license_type)
                .arg(expiration_date);
            LOG_STATUS(msg);
        }
        
        return out;
    }

    License license_beat()
    {
        std::string expiration;
        License ok = validate_license(expiration);
        if (ok != License::kValid)
        {
            if (ok == License::kExpired)
            {
                fl_alert("License expired on %s. Please enter new license.",
                         expiration.c_str());
                Fl::check();
            }

#ifdef _WIN32
            std::string helper = rootpath() + "/bin/license_helper.exe";
#else
            std::string helper = rootpath() + "/bin/license_helper";
#endif
#ifdef __APPLE__
            // This is needed for macOS installed bundle.
            if (!file::isReadable(helper))
            {
                helper = rootpath() + "/../Resources/bin/license_helper";
            }
#endif
            int ret = os::exec_command(helper.c_str());
            if (ret == 0)
            {
                License ok = validate_license(expiration);
                if (ok == License::kInvalid)
                {
                    fl_alert("%s", _("Invalid license. Entering demo mode"));
                    Fl::check();
                    App::demo_mode = true;
                }
                else if (ok == License::kExpired)
                {
                    fl_alert("%s", _("License expired. Entering demo mode"));
                    Fl::check();
                    App::demo_mode = true;
                }
            }
            else
            {
                App::demo_mode = true;
            }
        }
        return ok;
    }
}
