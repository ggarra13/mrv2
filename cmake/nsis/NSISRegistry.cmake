# Extra install commands for NSIS

	set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		WriteRegStr HKCR 'mrv2' '' 'mrv2'
		WriteRegStr HKCR 'mrv2\\\\shell' '' 'open'
		WriteRegStr HKCR 'mrv2\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\mrv2.exe \\\"%1\\\"'
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext.nsh
	Section \\\"File Associations\\\"
		"
		)


	set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		DeleteRegKey HKCR 'mrv2'
	SectionEnd
	!addincludedir ${PROJECT_SOURCE_DIR}/../cmake/nsis
	!include fileext_uninstall.nsh
	Section \\\"un.File Associations\\\"
	"
	)
