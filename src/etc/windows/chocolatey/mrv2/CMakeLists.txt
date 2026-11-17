cmake_minimum_required(VERSION 3.16)
project(mrv2-chocolatey NONE)

# ---------------------------------------------------------------------------
# Inputs
# ---------------------------------------------------------------------------
set(MRV2_NAME    "" CACHE STRING "Project/package name: mrv2 or vmrv2")
set(MRV2_VERSION "" CACHE STRING "Version to package, e.g. 1.4.0 (no leading 'v')")
set(MRV2_GITHUB_REPO "" CACHE STRING "GitHub 'owner/repo' (default: ggarra13/<MRV2_NAME>)")
set(MRV2_SILENT_ARGS "/S" CACHE STRING "Silent switches of the installer (/S for NSIS)")
set(MRV2_DISPLAY_NAME "" CACHE STRING "DisplayName in Add/Remove Programs (default: MRV2_NAME)")

# Per-architecture overrides (optional)
set(MRV2_URL_AMD64     "" CACHE STRING "Override download URL of the amd64 installer")
set(MRV2_URL_AARCH64   "" CACHE STRING "Override download URL of the aarch64 installer")
set(MRV2_SHA256_AMD64   "" CACHE STRING "SHA256 of the amd64 installer (downloaded if empty)")
set(MRV2_SHA256_AARCH64 "" CACHE STRING "SHA256 of the aarch64 installer (downloaded if empty)")

if(NOT MRV2_NAME)
    message(FATAL_ERROR "Pass -DMRV2_NAME=mrv2 (or vmrv2)")
endif()
if(NOT MRV2_VERSION)
    message(FATAL_ERROR "Pass -DMRV2_VERSION=x.y.z")
endif()

set(mrv2_DESCRIPTION "OpenGL backend")
if (MRV2_NAME STEQUAL "vmrv2")
    set(mrv2_DESCRIPTION "Vulkan backend")
endif()

set(mrv2_NAME    ${MRV2_NAME})
set(mrv2_VERSION ${MRV2_VERSION})

if(NOT MRV2_GITHUB_REPO)
    set(MRV2_GITHUB_REPO "ggarra13/mrv2")
endif()

if (MRV2_NAME STREQUAL "vmrv2")
    set(mrv2_DESCRIPTION "Vulkan backend")
elseif (MRV2_NAME STREQUAL "mrv2")
    set(mrv2_DESCRIPTION "OpenGL backend")
else()
    message(FATAL_ERROR "Invalid MRV2_NAME (use mrv2 or vmrv2)")
endif()

if(NOT MRV2_DISPLAY_NAME)
    set(MRV2_DISPLAY_NAME "${MRV2_NAME}")
endif()

# ---------------------------------------------------------------------------
# Resolve URL + SHA256 for each architecture
# ---------------------------------------------------------------------------
# Sets MRV2_URL_<ARCH> and MRV2_SHA256_<ARCH> (ARCH uppercased) in the caller.
function(mrv2_resolve_installer ARCH)
    string(TOUPPER "${ARCH}" A)

    set(url "${MRV2_URL_${A}}")
    if(NOT url)
        # Adjust to match the real asset name on https://github.com/<repo>/releases
        set(url "https://github.com/${MRV2_GITHUB_REPO}/releases/download/v${MRV2_VERSION}/${MRV2_NAME}-v${MRV2_VERSION}-Windows-${ARCH}.exe")
    endif()

    #
    # On Windows, there's no mrv2 aarch64 as OpenGL is deprecated.
    #
    if (MRV2_NAME STREQUAL "mrv2" AND
	    ARCH STREQUAL "aarch64")
	return()
    endif()

    set(sha "${MRV2_SHA256_${A}}")
    if(NOT sha)
        message(STATUS "Downloading ${url} to compute SHA256 ...")
        set(exe "${CMAKE_BINARY_DIR}/${MRV2_NAME}-${MRV2_VERSION}-${ARCH}.exe")
        file(DOWNLOAD "${url}" "${exe}" STATUS st SHOW_PROGRESS)
        list(GET st 0 code)
        if(NOT code EQUAL 0)
            message(FATAL_ERROR "Download failed for ${ARCH}: ${st}")
        endif()
        file(SHA256 "${exe}" sha)
    endif()

    message(STATUS "${MRV2_NAME} ${MRV2_VERSION} (${ARCH})  sha256=${sha}")
    set(MRV2_URL_${A}    "${url}" PARENT_SCOPE)
    set(MRV2_SHA256_${A} "${sha}" PARENT_SCOPE)
endfunction()

mrv2_resolve_installer(amd64)
mrv2_resolve_installer(aarch64)


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
