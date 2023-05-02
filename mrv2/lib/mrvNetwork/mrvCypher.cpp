// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCypher.h"

namespace
{
    const std::string kKey = "mrv2 is simple and great!";
}

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
        xor_cipher(plainText, kKey, out);
        return out;
    }

    std::string decode_string(const std::string& encodedText)
    {
        std::string out;
        xor_cipher(encodedText, kKey, out);
        return out;
    }
} // namespace mrv
