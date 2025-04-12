include(ExternalProject)

set(CURL_GIT_REPOSITORY "https://github.com/curl/curl.git")
set(CURL_GIT_TAG "curl-8_4_0")

set(CURL_ARGS
    -DBUILD_CURL_EXE=OFF
    -DBUILD_TESTING=OFF
    -DCURL_USE_OPENSSL=ON
    -DCURL_USE_LIBSSH2=ON
    -DCURL_DISABLE_LDAP=ON
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    CURL
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/CURL
    DEPENDS Libssh2 ${OpenSSL} ZLIB
    GIT_REPOSITORY ${CURL_GIT_REPOSITORY}
    GIT_TAG ${CURL_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${CURL_ARGS})
