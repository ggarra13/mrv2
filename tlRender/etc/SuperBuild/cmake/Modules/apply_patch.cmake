# apply_patch.cmake
#
# Apply a patch to a dependency's source, and do nothing if it is already
# applied.
#
# Usage:
#   cmake
#     -DPATCH_SOURCE_DIR=/path/to/source
#     -DPATCH_FILE=/path/to/patch
#     -DGIT_EXECUTABLE=/path/to/git
#     -P apply_patch.cmake
#
# The patch step runs again whenever its stamp is cleared, which is not always
# accompanied by a fresh clone, and "git apply" fails on a tree it has already
# been applied to. Asking whether it reverses tells the two apart: a tree that
# the patch can be taken back out of is a tree it is already in.

if(NOT DEFINED PATCH_SOURCE_DIR)
    message(FATAL_ERROR "PATCH_SOURCE_DIR is required")
endif()

if(NOT DEFINED PATCH_FILE)
    message(FATAL_ERROR "PATCH_FILE is required")
endif()

if(NOT DEFINED GIT_EXECUTABLE)
    message(FATAL_ERROR "GIT_EXECUTABLE is required")
endif()

if(NOT IS_DIRECTORY "${PATCH_SOURCE_DIR}")
    message(FATAL_ERROR
        "PATCH_SOURCE_DIR does not exist or is not a directory: "
        "${PATCH_SOURCE_DIR}"
    )
endif()

if(NOT EXISTS "${PATCH_FILE}")
    message(FATAL_ERROR
        "PATCH_FILE does not exist: ${PATCH_FILE}"
    )
endif()

if(NOT EXISTS "${GIT_EXECUTABLE}")
    message(FATAL_ERROR
        "GIT_EXECUTABLE does not exist: ${GIT_EXECUTABLE}"
    )
endif()

# If the patch applies in reverse, it is already present.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${PATCH_FILE}"
    WORKING_DIRECTORY "${PATCH_SOURCE_DIR}"
    RESULT_VARIABLE REVERSE_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)

if(REVERSE_RESULT EQUAL 0)
    message(STATUS
        "Patch already applied: ${PATCH_FILE}"
    )
    return()
endif()

# Make sure the patch can be applied before modifying the source tree.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --check "${PATCH_FILE}"
    WORKING_DIRECTORY "${PATCH_SOURCE_DIR}"
    RESULT_VARIABLE CHECK_RESULT
    OUTPUT_VARIABLE CHECK_OUTPUT
    ERROR_VARIABLE CHECK_ERROR
)

if(NOT CHECK_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Patch cannot be applied and does not appear to be already applied:\n"
        "  Source: ${PATCH_SOURCE_DIR}\n"
        "  Patch:  ${PATCH_FILE}\n"
        "\n"
        "${CHECK_OUTPUT}"
        "${CHECK_ERROR}"
    )
endif()

# Apply the patch.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply "${PATCH_FILE}"
    WORKING_DIRECTORY "${PATCH_SOURCE_DIR}"
    RESULT_VARIABLE APPLY_RESULT
    OUTPUT_VARIABLE APPLY_OUTPUT
    ERROR_VARIABLE APPLY_ERROR
)

if(NOT APPLY_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Failed to apply patch:\n"
        "  Source: ${PATCH_SOURCE_DIR}\n"
        "  Patch:  ${PATCH_FILE}\n"
        "\n"
        "${APPLY_OUTPUT}"
        "${APPLY_ERROR}"
    )
endif()

message(STATUS "Applied patch: ${PATCH_FILE}")
