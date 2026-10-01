# Apply a patch to a dependency's source, and do nothing if it is already
# applied.
#
# The patch step runs again whenever its stamp is cleared, which is not always
# accompanied by a fresh clone, and "git apply" fails on a tree it has already
# been applied to. Asking whether it reverses tells the two apart: a tree that
# the patch can be taken back out of is a tree it is already in.
#
# Run with -DPATCH_SOURCE_DIR=... -DPATCH_FILE=... -DGIT_EXECUTABLE=...

execute_process(
    COMMAND ${GIT_EXECUTABLE} apply --reverse --check ${PATCH_FILE}
    WORKING_DIRECTORY ${PATCH_SOURCE_DIR}
    RESULT_VARIABLE alreadyApplied
    OUTPUT_QUIET ERROR_QUIET)
if(alreadyApplied EQUAL 0)
    message(STATUS "${PATCH_FILE} is already applied")
    return()
endif()

execute_process(
    COMMAND ${GIT_EXECUTABLE} apply ${PATCH_FILE}
    WORKING_DIRECTORY ${PATCH_SOURCE_DIR}
    RESULT_VARIABLE result
    ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    # Most likely the change has landed upstream, or the dependency's version
    # moved and the file the patch expects is no longer what is there. Either
    # way the patch has to be looked at rather than worked around.
    message(FATAL_ERROR
        "Could not apply ${PATCH_FILE}:\n${error}\n"
        "If the version moved, re-make the patch against the new source. If "
        "the change is upstream now, drop the patch and this step with it.")
endif()
message(STATUS "${PATCH_FILE} applied")
