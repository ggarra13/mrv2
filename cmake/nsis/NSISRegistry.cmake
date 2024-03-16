# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Extra install commands for NSIS


    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext.nsh
	Section \\\"-Dummy\\\"
		"
    )


    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.Dummy\\\"
	"
    )
