Manifests currently unused as vcpkg is behind latest releases and has issues with patching.
Also, some of the libraries we use are not in vcpkg.

We do use vcpkg in Windows builds for libintl/libiconv, and openssl/libcrypto both of which are hard to compile manually.
