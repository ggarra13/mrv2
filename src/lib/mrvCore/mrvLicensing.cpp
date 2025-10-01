#include "mrvApp/mrvApp.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvLicensing.h"
#include "mrvCore/mrvOS.h"

#include <Poco/Net/DNS.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HostEntry.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Exception.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timespan.h>
#include <Poco/Path.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/x509_vfy.h>

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
}

namespace mrv
{
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

    License validate_license(std::string& expiration_date)
    {
        try
        {
            // --- Configuration ---
            std::string serverHost = "srv1037957.hstgr.cloud";
            int serverPort = 443;
            std::string machine_id = get_machine_id();

            // --- Load verify key (Base64) ---
            std::string verify_key_b64 = "V/7Jp4Ngx1g1sSiovQgxl9utH4oylriTMZFNNLHefGU=";

            // --- Build JSON request ---
            std::string requestBody = "{\"machine_id\":\"" + machine_id + "\",\"plan\":\"Pro\"}";

            std::string caLocation = mrv::rootpath() + "/certs/cacert.pem";

            if (!file::isReadable(caLocation))
            {
                LOG_STATUS(caLocation << " is not readable");
            }
            
// #ifdef __linux___
//             caLocation = "/etc/ssl/certs/ca-certificates.crt";
// #endif

// #ifdef __APPLE__
//             caLocation = "/usr/local/etc/openssl@3/cert.pem";
// #endif
            
            Poco::Net::Context::Ptr context = new Poco::Net::Context(
                Poco::Net::Context::CLIENT_USE,
                "",    // privateKeyFile
                "",    // certificateFile
                caLocation,    // caLocation ("" = system default, if available)
                Poco::Net::Context::VERIFY_RELAXED, // or VERIFY_STRICT
                9,     // verificationDepth
                false, // load default CA location
                "ALL"
                );

            // --- HTTP POST to /request_license ---
            
            Poco::Net::HTTPSClientSession session(serverHost, serverPort, context);
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/request_license", Poco::Net::HTTPMessage::HTTP_1_1);
            request.setContentType("application/json");
            request.setContentLength(requestBody.length());

            std::ostream& os = session.sendRequest(request);
            os << requestBody;

            Poco::Net::HTTPResponse response;
            std::istream& rs = session.receiveResponse(response);

            std::ostringstream oss;
            Poco::StreamCopier::copyStream(rs, oss);
            std::string respStr = oss.str();
        
            if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
            {
                // Parse JSON error message
                Poco::JSON::Parser parser;
                Poco::Dynamic::Var result = parser.parse(respStr);
                Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
                if (obj->has("detail")) {
                    LOG_ERROR(obj->getValue<std::string>("detail"));
                } else {
                    LOG_ERROR(respStr);
                }
                return License::kInvalid;        
            }

            // --- Parse JSON response ---
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(respStr);
            Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
    
            Poco::JSON::Object::Ptr payloadObj = obj->getObject("payload");
            std::string signature = obj->getValue<std::string>("signature");

            // Manually build payload JSON string in specific order with spaces (to match signed format in Python)
            std::string payload_machine_id = payloadObj->getValue<std::string>("machine_id");
            std::string expires_at = payloadObj->getValue<std::string>("expires_at");
            std::string plan = payloadObj->getValue<std::string>("plan");

            std::string payloadJson = "{\"machine_id\": \""
                                      + payload_machine_id + "\", " 
                                      + "\"expires_at\": \"" + expires_at + "\", " 
                                      + "\"plan\": \"" + plan + "\"" 
                                      + "}";

            // -------------------------
            // Verify license
            // -------------------------
            bool valid = verify_ed25519(verify_key_b64, payloadJson, signature);

            if (!valid)
            {
                LOG_ERROR("❌ Invalid signature");
                return License::kInvalid;
            }

            expiration_date = expires_at;

            // Check expiration
            std::tm tm = {};
            std::istringstream ss(expires_at);
            ss >> std::get_time(&tm, "%Y-%m-%d");
            std::time_t exp_time = std::mktime(&tm);

            std::time_t now = std::time(nullptr);
            if (now > exp_time) {
                LOG_ERROR("License expired");
                return License::kExpired;
            }

            // Check machine_id
            if (payload_machine_id != machine_id) {
                LOG_ERROR("machine_id does not match");
                return License::kInvalid;
            }

            std::string msg = string::Format(_("Your license will expire on {0}.")).arg(expires_at);
            LOG_STATUS(msg);

            return License::kValid;
        }
        catch (const Poco::Exception& ex) {
            LOG_ERROR("Request failed: " << ex.displayText());
        }
        catch (const std::exception& ex) {
            LOG_ERROR(ex.what());
        }
        return License::kInvalid;
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
                    fl_alert("Invalid license. Entering demo mode");
                    Fl::check();
                    App::demo_mode = true;
                }
                else if (ok == License::kExpired)
                {
                    fl_alert("License expired. Entering demo mode");
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
