
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
		and python version and os compilers and linkers.
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

src/icons
	Original SVG icons in .svg format.

tlRender/lib/
	tlCore		   - Core tlRender classes.
	tlDevice  	   - Device classes (NDI)
	tlDraw        	   - Annotation Drawing classes.
	tlGL	       	   - Base OpenGL classes
	tlIO	       	   - I/O Plugin classes.
	tlTimeline     	   - OpenTimelineIO Timeline classes.
	tlTimelineGL	   - OpenGL Timeline drawing and tonemapping classes.
	tlTimelineUI	   - OpenGL and Vulkan Timeline drawing/editing classes.
	tlTimelineVK	   - Vulkan Timeline drawing and HDR classes.
	tlUI               - Legacy GLFW drawing classes (may be removed)
	tlVk		   - Base Vulkan classes
	

src/lib:

	mrvApp		- Main application, command-line parsing and settings.
	                  ----------------------------------------------------
			  mrvApp             - Main application class (refactor)
			  mrvFilesModel      - Files data
			  mrvGlobals         - Global flags
			  mrvMainControl     - Main controller
			  mrvOpenSeparateAudioDialog - move to mrvUI
			  mrvPlaylistModel   - Playlist model
			  mrvSettingsObject  - Settings storage
			  mrvStdAnyHelper    - Auxiliary functions for
			  		       std::any class
	mrvBaseAp	- Base classes for an application
			  -------------------------------
			  mrvBaseApp	     - Main base application
			  mrvCmdLine	     - Command line parser
			  mrvCmdLineInline   - Inline functions for command-line

	mrvCore    	- Core functionality.
			  ------------------
			  mrvActionMode.h    - UI Action mode (refactor)
			  mrvBackend.h       - Defines and namespaces for UI
			  		       backend
					       Currently OpenGL and Vulkan.
			  mrvColorAreaInfo.h - Color area selection information
			  		       (refactor)
			  mrvColorSpaces.h   - Transform color among different
			  		       color spaces
					       (depends on Imf::Chromaticities)
			  mrvCPU.cpp         - get CPU capabilities
			  mrvEnv.h/cpp       - Windows wchar versions of Unix
			  		       setenv and unsetenv.
			  mrvFile.h/cpp      - Query functions for file types
			  		       (rename to mrvFileType.h)
			  mrvFileManager.h/mm/cpp - Open files in default
			  			    file manager.
						    (refactor to mrvOS)
			  mrvFonts.h/cpp     - list and compare fonts
			  mrvHome.h/cpp      - list paths to several useful
			 		       directories like preferences,
					       user's home, etc.
		  	  mrvHotkey.h/cpp    - Hotkey class and HotkeyEntry
			  		       class to store user's hotkeys
					       (refactor to mrvUI)
			  mrvI8N.h	     - Handle internationalization with
			  		       gettext / libintl
					       (why do Windows needs turning
					       	off macros?)
			  mrvImageOps.h/cpp  - Functions to operate on images.
			  		      (refactor to mrvImage/ directory)
			  mrvLocale.h/cpp    - RAII class to switch locale
			  		       settings (usually LC_NUMERIC)
			  mrvMath.h	     - Auxiliary math classes
			  mrvMedia.h         - Static strings for ocio ICS
			  		       defaults (refactor to mrvUI)
			  mrvMemory.h        - Function used to determine used
			  		       and virtual memory on all
					       OSes. (refactor to mrvOS)
			  mrvMesh.h          - Functions to create auxiliary
			  		       meshes compatible with tlRender.
			  mrvOS.h            - Common functions like execv on
			  		       all platforms.  (Split and
					       refactor between mrvOS/ and
					       mrvUI/)
			  mrvOrderedMap.h   - like std::map but keeps the order
			  		      of insertion.
			  mrvPathMapping.h/cpp
					    - Implements path mapping algorithm.
			  mrvPixelConverter.h
					    - Template functions to convert
					      pixels.
					      (Refactor to mrvImage/)
			 mrvRoot.h         - Finds root directory from argv[0]
			 		     and sets MRV2_ROOT environment
					     variable to it
					     (refactor to mrvOS/)
			mrvSequence.h	  - Sorts sequence based on basename,
					    number, view and extension.
					    (merge into tlCore/Path.h)
			mrvSignalHandler.h - Installs signal handler
					   (refactor to mrvOS/)
			mrvStackTrace.h	   - Spit out a stack trace
					     (refactor to mrvOS/)
			mrvString.h        - String auxiliary functions
					     (some overlap with tlRender now)
			mrvTimeObject.h/cpp - Timecode functions.
			mrvUtil.h	    - Some utils that don't fit anywhere
			mrvWait.h           - Wait some time in milliseconds
					      refreshing the UI
					      (refactor to mrvFLTK/)		   
			  
	mrvEdit    	- Editing callbacks and functions.
			  mrvCreateEDLFromFiles - given a list of files, create
			  			  EDL.
			  mrvEditCallbacks      - FLTK Editing callbacks
			  mrvEditMode		- Editing mode enum class
			  mrvEditUtil		- Utilities for editing.
			  
	mrvIcons   	- Binary SVG icons for faster loading.
	
	mrvFl      	- mrv2's classes for different functions

			  mrvCallbacks          - FLTK callbacks
			  			  (refactor to mrvFLTK)
			  mrvColorSchemes	- FLTK color themes
			  			  (refactor to mrvFLTK)
			  mrvContextObject	- FLTK context object
			  			  (updates tlRender's observers)
			  mrvConvertImage       - converts images
			  			  (refactor to mrvImage)
			  mrvFileRequester	- FLTK entry point functions
			  			  for file requester.
			  			  (refactor to mrvFLTK)
			  mrvHotkey		  Hotkey UI functions
			  			  (refactor to mrvUI)
			  mrvIO			  Main logging and output.
			  			  (refactor to mrvCore?)
			  mrvInit		  Initialize tlRender.
			  mrvLanguages		  Handle internationalization.
			  			  (refactor to mrvCore?)
			  mrvLaserFadeData	  Laser fade data
			  			  (refactor to mrvViewport)
			  mrvOCIO		  OCIO changing and presets
			  			  (refactor to mrvUI?)
			  mrvPathMapping	  FLTK Path Mapping functions.
			  mrvPreferences	  Load/Save/Init preferences
			  			  (refactor to mrvUI)
			  mrvSave		  FLTK Save entry functions
			  			  (refactor to mrvFLTK)
			  mrvSaveImage		  OpenGL save image function
			  			  (rename to mrvSaveImageGL)
			  mrvSaveImageVk	  Vulkan save image function
			  mrvSaveMovie		  OpenGL save movie function
			  			  (rename to mrvSaveMovieGL)
			  mrvSaveMovieVk	  Vulkan save movie function
			  mrvSaveOptions	  Options for saving movies
			  			  (refactor to mrvOptions)
			  mrvSession		  Sessions saving and loading
			  mrvStereo3DAux	  Match one layer to anoter one
			  			  in OpenEXR files.
						  (refactor to other layer
						   file)
			 mrvTimelinePlayer	 Main timeline player
			 			 (where to put it?)
			 mrvUSD			 Send USD flags
			 			 (refactor)
			 mrvVersioning		 Versioning functions
			 			 (refactor to mrvUI)

	mrvFlmm    	- Mathias Melcher's Flmm_ColorA_Chooser.
			  Keep as is.

	mrvFLU     	- mrv2's custom file chooser (based on origina FLU
	             	  file chooser).  Needs refactoring and code cleanup.

        mrvGL      	- OpenGL driver classes.
			  Keep as is

        mrvHDR		- HDR support libraries for 'hdr' utility.
			  Keep as is.
			  
        mrvHDRWidgets   - Main UI of 'hdr' utility.
			  (refactor to mrvHDR)
			  
	mrvNetwork 	- Network classes.
			  Keep as is.
			  
	mrvOptions 	- Options classes for mrv2's custom code.
			  Keep as is.
			  
	mrvPanels  	- All of mrv2's Docking Panels/Windows.
			  Keep as is.
			  
	mrvPDF     	- PDF exporting classes.  Needs updating to FLTK's new
		          PDF classes.
			  Keep as is.
			  
	mrvPy		- Python (pybind11) code.  Must remove mrv2 namespace.
			  Keep as is.  Some files renaming perhasp.
			  
	mrvUI      	- Menus, Desktop and SVG loading functions.
			  Keep as is, move a bunch of files here too.

			  mrvDesktop and mrvMonitor move to mrvCore, so
			  it can be used by mrvUI without repeating code.

	mrvViewport	- Viewport functions common to all backends.
			  Keep as is.

	mrvWidgets 	- FLTK custom widgets and main fluid UI (.fl) files.
	             	  .fl files should be refactored to mrvUI?
			  Keep as is.

src/hdr:
	Main entry point for 'hdr' NDI utility.

src/main:
	main.cpp     - Main entry point and python module initialization.
		       
