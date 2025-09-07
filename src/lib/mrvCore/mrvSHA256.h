/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.
*********************************************************************/

#ifndef SHA256_H
#define SHA256_H

/*************************** HEADER FILES ***************************/
#include <cstddef>
#include <cstdlib>
#include <string>

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/

typedef struct {
	unsigned char data[64];
	unsigned int datalen;
	unsigned long long bitlen;
	unsigned int state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len);
void sha256_final(SHA256_CTX *ctx, unsigned char hash[]);

namespace mrv
{
    enum class License
    {
        kValid = 0,
        kInvalid = 1,
        kExpired = 2,
    };
    
    std::string sha256(const std::string& input);
    License validate_license(std::string& expiration_date,
                             const std::string& secret_salt = "goblydock");
    void license_beat();
}

#endif   // SHA256_H
