
Build System
============

. (mrv2's root dir):

	runme.sh
		Main build script.  It will perform the full build and
		update all the dependencies, but if run with a full build, it
		can take a while to run.
	runmeq.sh
		Main script to build just mrv2 once runme.sh was run.
		Fast way to do iterations on mrv2's source code.
	runmet.sh
		Main script to build mrv2, tlRender and FLTK all of
		which change often.  In order to use this script, you
		must manually use git to pull the latest tlRender or FLTK (or
		whatever tag you want to build against).		

bin/:

	Auxiliary scripts for doing multiple auxiliary tasks, including
	Windows' pre-flight compilations.
	They should be self-explanatory with their names.  If not, open
	them in your text editor.  They are all documented what they do.

	There are also several runme_* scripts to compile custom versions
	of mrv2 with less features.
	

certificates/:
	
	Windows and macOS certificates for signing the files and installer.
	Currently, only used on Windows for a self-signed installer.

etc/:

	build_cores.sh
		Functions used to detect the number of cores on
		all platforms.
	build_dir.sh
		Set up paths for building.
	build_end.sh
		Final script called after build is done to add symlinks.
	compile_windows_dlls.sh
		Windows' MSys installs and pre-flight compilation runs.
	functions.sh
		Auxiliary functions to extract mrv2's version, cmake's version,
		and python version.
	install_cmake.sh
		Auxiliary function to install cmake.  Not used but kept in case
		cmake gets broken on GitHub actions as it happened before.
	parse_args.sh
		Main command-line parsing script for all runme_* scripts.
	runme_nolog.sh
		Main entry point to cmake's build process.  Will set up 
		defaults to all cmake variables and log them to the console.
		No logging is done from this script.
	sphinx_install.sh
		Auxiliary script used to install python's sphinx modules for
		documenting mrv2.
	windows_prepare.sh
		MSys preparation for building under MSVC.  Mainly, rename
		hardly used /usr/bin/link.exe as it conflicts with MSVC's 
		link.exe linker.
	windows_signing_installer.sh
		Windows' signing of the installer when *I* compile the code.

Development Instructions
========================

mrv2/lib:

	mrvApp     - Main application, command-line parsing and settings.
	mrvCore    - Core functionality.
	mrvDraw    - Annotation Drawing classes.
	mrvEdit    - Editing callbacks and functions.
	mrvFl      - mrv2's classes for different functions 
	             (needs refactoring).
	mrvFlmm    - Mathias Melcher's Flmm_ColorA_Chooser.
	mrvFLU     - mrv2's custom file chooser (based on origina FLU
	             file chooser).  Needs refactoring and code cleanup.
	mrvGL      - OpenGL driver classes.
	mrvNetwork - Network classes for machi
	mrvOptions - Options classes for mrv2's custom code.
	mrvPanels  - All of mrv2's Docking Panels/Windows.
	mrvPDF     - PDF exporting classes.  Needs updating to FLTK's new
		         PDF classes (don't work under Windows 8.1 thou)?
	mrvPy      - Python (pybind11) code.  Must remove mrv2 namespace.
	mrvUI      - Menus, Desktop and SVG loading functions. 
	mrvWidgets - FLTK custom widgets and main fluid UI (.fl) files.
	             .fl files should be refactored to mrvUI?

mrv2/src:

	main.cpp     - Main entry point and python module initialization.
