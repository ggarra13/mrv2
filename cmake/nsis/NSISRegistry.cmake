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
# Human readable version of hdr.
#
set(hdr_NAME "hdr NDI Viewer")
set(hdr_VERSION_NAME "${hdr_NAME} (v${mrv2_VERSION})")
set(hdr_LATEST_NAME "${hdr_NAME} Latest (currently: v${mrv2_VERSION})")

#
# The name of mrv2's hard-linked versioned command
#
set(mrv2_KEY "mrv2-v${mrv2_VERSION}.exe" )
set(hdr_KEY "hdr-v${mrv2_VERSION}.exe" )

#
# The name of mrv2's main exe commands.
#
set(mrv2_EXE "mrv2.exe" )
set(hdr_EXE "hdr.exe" )



# Extra install commands for NSIS
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "

        StrCmp '$INSTALL_DESKTOP' '1' 0 +2
           CreateShortCut '$DESKTOP\\\\hdr-64 v${mrv2_VERSION}.lnk' '$INSTDIR\\\\bin\\\\hdr.exe'

        #
        # Remove v1.0.9 and older registry entry
        #
	DeleteRegKey HKCR 'mrv2'
        
	#
        # Write out a friendly name with the version number besides the opening
        # command and the icon.
        #
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}' 'FriendlyAppName' '${mrv2_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open' '' 'Play with ${mrv2_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_KEY} \\\"%1\\\"'

        #
        # This create the actual file association command.
        #
        WriteRegStr HKCR '${mrv2_KEY}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_KEY} \\\"%1\\\"'



	#
        # Write out a friendly name for 'latest' version.
        #
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}' 'FriendlyAppName' '${mrv2_LATEST_NAME}'
        WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${mrv2_EXE},0'
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell\\\\Open' '' 'Play with ${mrv2_LATEST_NAME}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_EXE}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_EXE} \\\"%1\\\"'

        #
        # This create the actual file association command.
        #
        WriteRegStr HKCR '${mrv2_EXE}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${mrv2_EXE} \\\"%1\\\"'

	#
        # Write out a friendly name with the version number besides the opening
        # command and the icon.
        #
	WriteRegStr HKCR 'Applications\\\\${hdr_KEY}' 'FriendlyAppName' '${hdr_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${hdr_KEY}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${hdr_KEY}\\\\shell\\\\Open' '' 'Play with ${hdr_VERSION_NAME}'
	WriteRegStr HKCR 'Applications\\\\${hdr_KEY}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${hdr_KEY} \\\"%1\\\"'

        #
        # This create the actual file association command.
        #
        WriteRegStr HKCR '${hdr_KEY}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${hdr_KEY} \\\"%1\\\"'



	#
        # Write out a friendly name for 'latest' version.
        #
	WriteRegStr HKCR 'Applications\\\\${hdr_EXE}' 'FriendlyAppName' '${hdr_LATEST_NAME}'
        WriteRegStr HKCR 'Applications\\\\${hdr_EXE}\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${hdr_EXE},0'
	WriteRegStr HKCR 'Applications\\\\${hdr_EXE}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${hdr_EXE}\\\\shell\\\\Open' '' 'Play with ${hdr_LATEST_NAME}'
	WriteRegStr HKCR 'Applications\\\\${hdr_EXE}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\${hdr_EXE} \\\"%1\\\"'

        #
        # This create the actual file association command.
        #
        WriteRegStr HKCR '${hdr_EXE}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${hdr_EXE} \\\"%1\\\"'

	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
        !define  mrv2_KEY ${mrv2_KEY}
        !define  mrv2_EXE ${mrv2_EXE}
        !define  hdr_KEY ${hdr_KEY}
        !define  hdr_EXE ${hdr_EXE}
	!include fileext.nsh

	Section \\\"-Dummy\\\"
		"
)


# Extra uninstall commands for NSIS
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "

        StrCmp '$INSTALL_DESKTOP' '1' 0 +2
           Delete '$DESKTOP\\\\hdr-64 v${mrv2_VERSION}.lnk'

        # Delete the versioned application and file association entry.
        DeleteRegKey HKCR 'Applications\\\\${mrv2_KEY}'
	DeleteRegKey HKCR '${mrv2_KEY}'

        # Delete the versioned application and file association entry.
        DeleteRegKey HKCR 'Applications\\\\${hdr_KEY}'
	DeleteRegKey HKCR '${hdr_KEY}'

	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.Dummy\\\"
	"
)
