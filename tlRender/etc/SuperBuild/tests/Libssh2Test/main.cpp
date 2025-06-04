#include <libssh2.h>

int main(int argc, char* argv[])
{
    int rc = libssh2_init(0);
    if (rc)
    {
        return 1;
    }
    libssh2_exit();
    return 0;
}
