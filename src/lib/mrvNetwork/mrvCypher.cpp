// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvCypher.h"

#include <sstream>
#include <iomanip>

namespace
{
    const std::string kKey = "mrv2 is simple and great!";
    bool cypher_enabled = true;
} // namespace

namespace mrv
{

    void xor_cipher_hex(
        const std::string& plaintext, const std::string& key,
        std::string& ciphertext)
    {
        int key_len = key.length();
        int plaintext_len = plaintext.length();
        for (int i = 0; i < plaintext_len; i++)
        {
            // XOR the current character with the corresponding character in
            // the key
            char xored = plaintext[i] ^ key[i % key_len];
        
            std::stringstream hex;
            hex << std::hex << std::setw(2) << std::setfill('0') << (static_cast<unsigned int>(static_cast<unsigned char>(xored)));
            ciphertext += hex.str();
        }
    }
    
    char hex_char_to_byte(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    }

    void xor_decipher_hex(
        const std::string& ciphertext, const std::string& key,
        std::string& plaintext)
    {
        plaintext.clear();
        int key_len = key.length();
        for (size_t i = 0; i < ciphertext.length(); i += 2)
        {
            unsigned char byte = (hex_char_to_byte(ciphertext[i]) << 4) |
                                 hex_char_to_byte(ciphertext[i + 1]);
            char decoded = byte ^ key[(i / 2) % key_len];
            plaintext += decoded;
        }
    }
    
    std::string encode_string(const std::string& plainText)
    {
        std::string out;
        if (cypher_enabled)
            xor_cipher_hex(plainText, kKey, out);
        else
            out = plainText;
        return out;
    }

    std::string decode_string(const std::string& encodedText)
    {
        std::string out;
        if (cypher_enabled)
            xor_decipher_hex(encodedText, kKey, out);
        else
            out = encodedText;
        return out;
    }

    void enable_cypher(const bool value)
    {
        cypher_enabled = value;
    }
} // namespace mrv
