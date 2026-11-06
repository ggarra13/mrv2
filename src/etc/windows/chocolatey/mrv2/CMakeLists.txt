cmake_minimum_required(VERSION 3.16)
project(mrv2-chocolatey NONE)

# ---------------------------------------------------------------------------
# Inputs
# ---------------------------------------------------------------------------
set(MRV2_NAME    "" CACHE STRING "Project/package name: mrv2 or wmrv2")
set(MRV2_VERSION "" CACHE STRING "Version to package, e.g. 1.4.0 (no leading 'v')")
set(MRV2_ARCH    "amd64" CACHE STRING "Architecture used in the installer name (amd64, arm64, ...)")
set(MRV2_GITHUB_REPO "" CACHE STRING "GitHub 'owner/repo' (default: ggarra13/<MRV2_NAME>)")
set(MRV2_URL     "" CACHE STRING "Override download URL of the installer")
set(MRV2_SHA256  "" CACHE STRING "SHA256 of the installer (computed by downloading it if empty)")
set(MRV2_SILENT_ARGS "/S" CACHE STRING "Silent switches of the installer (/S for NSIS)")
set(MRV2_DISPLAY_NAME "" CACHE STRING "DisplayName in Add/Remove Programs (default: MRV2_NAME)")

if(NOT MRV2_NAME)
    message(FATAL_ERROR "Pass -DMRV2_NAME=mrv2 (or wmrv2)")
endif()
if(NOT MRV2_VERSION)
    message(FATAL_ERROR "Pass -DMRV2_VERSION=x.y.z")
endif()

set(mrv2_NAME    ${MRV2_NAME})
set(mrv2_VERSION ${MRV2_VERSION})
set(mrv2_ARCH    ${MRV2_ARCH})

if(NOT MRV2_GITHUB_REPO)
    set(MRV2_GITHUB_REPO "ggarra13/mrv2")
endif()
if(NOT MRV2_DISPLAY_NAME)
    set(MRV2_DISPLAY_NAME "${MRV2_NAME}")
endif()

# Adjust to match the real asset name on https://github.com/<repo>/releases
if(NOT MRV2_URL)
    set(MRV2_URL
        "https://github.com/${MRV2_GITHUB_REPO}/releases/download/v${MRV2_VERSION}/${MRV2_NAME}-v${MRV2_VERSION}-Windows-${MRV2_ARCH}.exe")
endif()

# Chocolatey picks url64bit on 64-bit systems (amd64/arm64) and url otherwise.
if(MRV2_ARCH MATCHES "64")
    set(MRV2_URL_KEY      "url64bit")
    set(MRV2_CHECKSUM_KEY "checksum64")
    set(MRV2_CHECKSUMTYPE_KEY "checksumType64")
else()
    set(MRV2_URL_KEY      "url")
    set(MRV2_CHECKSUM_KEY "checksum")
    set(MRV2_CHECKSUMTYPE_KEY "checksumType")
endif()

if(NOT MRV2_SHA256)
    message(STATUS "Downloading ${MRV2_URL} to compute SHA256 ...")
    set(_exe "${CMAKE_BINARY_DIR}/${MRV2_NAME}-${MRV2_VERSION}-${MRV2_ARCH}.exe")
    file(DOWNLOAD "${MRV2_URL}" "${_exe}" STATUS _st SHOW_PROGRESS)
    list(GET _st 0 _code)
    if(NOT _code EQUAL 0)
        message(FATAL_ERROR "Download failed: ${_st}")
    endif()
    file(SHA256 "${_exe}" MRV2_SHA256)
endif()
message(STATUS "${MRV2_NAME} ${MRV2_VERSION} (${MRV2_ARCH})  sha256=${MRV2_SHA256}")

# ---------------------------------------------------------------------------
# Stage the package
# ---------------------------------------------------------------------------
set(STAGE "${CMAKE_BINARY_DIR}/stage")
set(OUT   "${CMAKE_BINARY_DIR}/package")

configure_file(mrv2.nuspec.in                    "${STAGE}/${MRV2_NAME}.nuspec"           @ONLY)
configure_file(tools/chocolateyinstall.ps1.in    "${STAGE}/tools/chocolateyinstall.ps1"   @ONLY)
configure_file(tools/chocolateyuninstall.ps1.in  "${STAGE}/tools/chocolateyuninstall.ps1" @ONLY)

# Our chocolateyuninstall.ps1 already runs the real uninstaller. Without this marker
# Chocolatey's auto-uninstaller runs a second time from its install-time registry
# snapshot and fails because Uninstall.exe is already gone.
file(WRITE "${STAGE}/.skipAutoUninstall"       "")
file(WRITE "${STAGE}/tools/.skipAutoUninstall" "")

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------
find_program(CHOCO_EXECUTABLE choco)

if(CHOCO_EXECUTABLE)
    add_custom_target(choco_pack ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT}"
        COMMAND "${CHOCO_EXECUTABLE}" pack "${STAGE}/${MRV2_NAME}.nuspec" --outputdirectory "${OUT}"
        WORKING_DIRECTORY "${STAGE}"
        COMMENT "Packing ${MRV2_NAME} ${MRV2_VERSION} Chocolatey package"
        VERBATIM)

    # Elevated shell:  cmake --build build --target choco_test_install
    add_custom_target(choco_test_install
        COMMAND "${CHOCO_EXECUTABLE}" install ${MRV2_NAME} --version ${MRV2_VERSION} --source "${OUT}" -y --force
        DEPENDS choco_pack
        VERBATIM)

    add_custom_target(choco_test_uninstall
        COMMAND "${CHOCO_EXECUTABLE}" uninstall ${MRV2_NAME} -y
        VERBATIM)

    # Needs `choco apikey` set first
    add_custom_target(choco_push
        COMMAND "${CHOCO_EXECUTABLE}" push "${OUT}/${MRV2_NAME}.${MRV2_VERSION}.nupkg" --source https://push.chocolatey.org/
        DEPENDS choco_pack
        VERBATIM)
else()
    message(WARNING "choco.exe not found: files were staged in ${STAGE} but 'choco_pack' is unavailable.")
endif()
