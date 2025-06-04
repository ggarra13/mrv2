#include <samplerate.h>

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "libsamplerate version: " << src_get_version() << std::endl;
    return 0;
}
