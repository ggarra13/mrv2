# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Extra install commands for NSIS

 # Sadly, we cannot version the command so that in
 # the Open With menus appears mrv2 v1.1.0, mrv2 v1.2.0, etc.
 
 set(mrv2_KEY "mrv2.exe" ) 

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	#
        # Write out a friendly name with the version number besides the opening
        # command and the icon.
        #
	WriteRegStr HKCR 'Applications' '' '${mrv2_KEY}'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}' 'FriendlyAppName' 'mrv2-v${mrv2_VERSION} Media Player'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell' '' ''
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open' '' 'Play with mrv2-v${mrv2_VERSION} Media Player'
	WriteRegStr HKCR 'Applications\\\\${mrv2_KEY}\\\\shell\\\\Open\\\\command' '' '$INSTDIR\\\\bin\\\\mrv2.exe \\\"%1\\\"'

	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
        !define  mrv2_KEY ${mrv2_KEY}
	!include fileext.nsh
	Section \\\"-Dummy\\\"
		"
   )


set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	DeleteRegKey HKCR 'Applications\\\\${mrv2_KEY}'
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.Dummy\\\"
	"
   )
