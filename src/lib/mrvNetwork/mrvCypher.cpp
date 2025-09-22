// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvCypher.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <sstream>
#include <vector>

namespace
{
    const std::string kKey = "mrv2 and vmrv2 are simple and great! Please don't crack it.";
    bool cypher_enabled = true;
} // namespace

namespace mrv
{
    namespace
    {
        // Helper to convert a single hex digit to its integer value
        unsigned char hexCharToByte(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            throw std::invalid_argument("Invalid hex character");
        }

        std::vector<unsigned char> hexStringToBytes(const std::string& hex) {
            if (hex.size() % 2 != 0)
                throw std::invalid_argument("Hex string must have even length");

            std::vector<unsigned char> bytes;
            bytes.reserve(hex.size() / 2);

            for (size_t i = 0; i < hex.size(); i += 2) {
                unsigned char high = hexCharToByte(hex[i]);
                unsigned char low  = hexCharToByte(hex[i + 1]);
                bytes.push_back((high << 4) | low);
            }
            return bytes;
        }
        
        void print_hex(const std::vector<unsigned char>& data) {
            for (unsigned char byte : data) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << std::endl;
        }
        
        std::vector<int> rc4_ksa(const std::string& key)
        {
            std::vector<int> S(256);
            std::iota(S.begin(), S.end(), 0); // Fill with 0-255
            
            int j = 0;
            for (int i = 0; i < 256; ++i) {
                j = (j + S[i] + key[i % key.length()]) % 256;
                std::swap(S[i], S[j]);
            }
            return S;
        }

        std::vector<unsigned char> rc4_prga(std::vector<int>& S,
                                            size_t length)
        {
            std::vector<unsigned char> keystream;
            int i = 0, j = 0;
            for (size_t k = 0; k < length; ++k) {
                i = (i + 1) % 256;
                j = (j + S[i]) % 256;
                std::swap(S[i], S[j]);
                int t = (S[i] + S[j]) % 256;
                keystream.push_back(static_cast<unsigned char>(S[t]));
            }
            return keystream;
        }

        std::vector<unsigned char> rc4_crypt(const std::string& data,
                                             const std::string& key)
        {
            std::vector<unsigned char> out;
            
            std::vector<int> S = rc4_ksa(key);
            std::vector<unsigned char> keystream = rc4_prga(S, data.length());

            for (size_t i = 0; i < data.length(); ++i) {
                out.push_back(static_cast<unsigned char>(data[i] ^ keystream[i]));
            }
            return out;
        }
    }
    
    void xor_cipher_hex(
        std::string& cipherText, const std::string& plaintext, const std::string& key)
    {
        std::vector<unsigned char> encrypted_data = rc4_crypt(plaintext, key);
        std::stringstream hex;
        for (auto xored : encrypted_data)
        {
            hex << std::hex << std::setw(2) << std::setfill('0') << (static_cast<unsigned int>(static_cast<unsigned char>(xored)));
        }
        cipherText = hex.str();
    }

    void xor_decipher_hex(
        std::string& plainText, const std::string& cipherText, const std::string& key) 
    {
        std::vector<unsigned char> encrypted_data = hexStringToBytes(cipherText);
            
        std::vector<int> S = rc4_ksa(key);
        std::vector<unsigned char> keystream = rc4_prga(S, encrypted_data.size());

        for (size_t i = 0; i < encrypted_data.size(); ++i) {
            plainText += static_cast<char>(static_cast<unsigned char>(encrypted_data[i]) ^ keystream[i]);
        }
    }
    
    std::string encode_string(const std::string& plainText)
    {
         
        std::string out;
        
        if (cypher_enabled)
            xor_cipher_hex(out, plainText, kKey);
        else
            out = plainText;
        return out;
    }

    std::string decode_string(const std::string& encodedText)
    {
        std::string out;
        if (cypher_enabled)
            xor_decipher_hex(out, encodedText, kKey);
        else
            out = encodedText;
        return out;
    }

    void enable_cypher(const bool value)
    {
        cypher_enabled = value;
    }
} // namespace mrv
