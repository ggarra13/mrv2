# Extra install commands for NSIS

	set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		WriteRegStr HKCR 'mrv2' '' 'mrv2'
		WriteRegStr HKCR 'mrv2\\\\shell' '' 'open'
		WriteRegStr HKCR 'mrv2\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\mrv2.exe \\\"%1\\\"'
	SectionEnd
	!addincludedir ${CMAKE_CURRENT_SOURCE_DIR}/nsis
	!include fileext.nsh
	Section \\\"empty\\\"
		"
		)


	set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		DeleteRegKey HKCR 'mrv2'
	SectionEnd
	!addincludedir ${CMAKE_CURRENT_SOURCE_DIR}/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.empty\\\"
	"
	)
