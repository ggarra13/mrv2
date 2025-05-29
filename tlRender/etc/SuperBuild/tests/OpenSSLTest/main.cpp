#include <openssl/ssl.h>

int main(int argc, char* argv[])
{
    const SSL_METHOD* method = TLSv1_2_client_method();
    return 0;
}
