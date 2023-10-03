// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCypher.h"

namespace
{
    const std::string kKey = "mrv2 is simple and great!";
    bool cypher_enabled = true;
} // namespace

namespace mrv
{

    void xor_cipher(
        const std::string& plaintext, const std::string& key,
        std::string& ciphertext)
    {
        int key_len = key.length();
        int plaintext_len = plaintext.length();
        for (int i = 0; i < plaintext_len; i++)
        {
            // XOR the current character with the corresponding character in
            // the key
            ciphertext += plaintext[i] ^ key[i % key_len];
        }
    }

    std::string encode_string(const std::string& plainText)
    {
        std::string out;
        if (cypher_enabled)
            xor_cipher(plainText, kKey, out);
        else
            out = plainText;
        return out;
    }

    std::string decode_string(const std::string& encodedText)
    {
        std::string out;
        if (cypher_enabled)
            xor_cipher(encodedText, kKey, out);
        else
            out = encodedText;
        return out;
    }

    void enable_cypher(const bool value)
    {
        cypher_enabled = value;
    }
} // namespace mrv
