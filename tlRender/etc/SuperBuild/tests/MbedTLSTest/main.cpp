#include <mbedtls/build_info.h>
#include <mbedtls/entropy.h>

int main(int argc, char* argv[])
{
    mbedtls_entropy_context context;
    mbedtls_entropy_init(&context);
    mbedtls_entropy_free(&context);
    return 0;
}
