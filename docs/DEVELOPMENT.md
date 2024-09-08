
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

	CMakeLists.txt
		Main CMakeLists.txt file.  Unless on Linux or macOS, it should
		not be run with cmake.  You should invoke runme.sh or one of
		the bin/runme_* scripts instead.
		
cmake/:

	build_python_windows.cmake
		Auxiliary script to build and install Python on Windows.
	copy_pdbs.cmake
		Auxiliary script to copy all Windows' pdbs used for debugging.
	documentation.cmake
		Auxiliary script with functions for documenting mrv2.
	dummy.cmake
		Empty script, needed by packaging.cmake so that variables would
		get passed to prepackage.cmake script.
	functions.cmake
		Auxiliary cmake functions.
	options.cmake
		CMake options for building mrv2.
	packaging.cmake
		CMake file used for packaging on all platforms.
	prepackage.cmake
		Script run in staging area run before packaging.cmake to
		clean-up files before packaging.
	translations.cmake
		CMake script used to deal with Natural Language translations.
	version.cmake
		Current mrv2 version.  All scripts extract the values from here.
		Note that there's currently no support to tag a version as a 
		beta one.

bin/:

	Auxiliary scripts for doing multiple auxiliary tasks, including
	Windows' pre-flight compilations.
	
	All scripts are to be run from mrv2's root directory, not from 
	within the bin directory.
	
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

src/lib:

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
	mrvNetwork - Network classes.
	mrvOptions - Options classes for mrv2's custom code.
	mrvPanels  - All of mrv2's Docking Panels/Windows.
	mrvPDF     - PDF exporting classes.  Needs updating to FLTK's new
		         PDF classes (don't work under Windows 8.1 thou)?
	mrvPy      - Python (pybind11) code.  Must remove mrv2 namespace.
	mrvUI      - Menus, Desktop and SVG loading functions. 
	mrvWidgets - FLTK custom widgets and main fluid UI (.fl) files.
	             .fl files should be refactored to mrvUI?

src/main:

	main.cpp     - Main entry point and python module initialization.
