# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Human readable version of mrv2.
#
set(mrv2_NAME "mrv2 Media Player")
set(mrv2_VERSION_NAME "${mrv2_NAME} (v${mrv2_VERSION})")
set(mrv2_LATEST_NAME "${mrv2_NAME} Latest (currently: v${mrv2_VERSION})")

#
# The name of mrv2's hard-linked versioned command
#
set(mrv2_KEY "mrv2-v${mrv2_VERSION}.exe" )

#
# The name of mrv2's main exe command.
#
set(mrv2_EXE "mrv2.exe" )



# Extra install commands for NSIS
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	#
        # Write out a friendly name with the version number besides the opening
        # command and the icon.
        #
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}' 'FriendlyAppName' '${mrv2_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open' '' 'Play with ${mrv2_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_KEY} \\\"%1\\\"'

	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}' 'FriendlyAppName' '${mrv2_LATEST_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell\\\\Open' '' 'Play with ${mrv2_LATEST_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_EXE} \\\"%1\\\"'

        #
        # This create the actual file association command.
        #
	DeleteRegKey HKCR '${mrv2_KEY}'
        WriteRegStr HKCR '${mrv2_KEY}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_KEY} \\\"%1\\\"'

	DeleteRegKey HKCR '${mrv2_EXE}'
        WriteRegStr HKCR '${mrv2_EXE}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_EXE} \\\"%1\\\"'

	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
        !define  mrv2_KEY ${mrv2_KEY}
        !define  mrv2_EXE ${mrv2_EXE}
	!include fileext.nsh
	Section \\\"-Dummy\\\"
		"
   )


# Extra uninstall commands for NSIS
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	DeleteRegKey HKCR 'Applications\\\\${mrv2_KEY}'
	DeleteRegKey HKCR 'Applications\\\\${mrv2_EXE}'
	DeleteRegKey HKCR '${mrv2_KEY}'
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.Dummy\\\"
	"
   )
