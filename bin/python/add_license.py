#!/usr/bin/env python
#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script adds licensing information to all .cpp, .h, .cmake, .sh, .py
#
# You must run it from the root of the mrv2 project.
#

import os
import glob
import re
import sys
import stat
import shutil

TLRENDER_DIRS = [
    "tlRender/lib/tlBaseApp",
    "tlRender/lib/tlCore",
    "tlRender/lib/tlIO",
    "tlRender/lib/tlGL",
    "tlRender/lib/tlTimeline",
    "tlRender/lib/tlTimelineGL",
    "tlRender/lib/tlTimelineUI",
    "tlRender/lib/tlDevice",
]

CPP_DIRS = [
    "src/hdr",
    "src/main",
    "src/lib/mrvApp",
    "src/lib/mrvCore",
    "src/lib/mrvWidgets",
    "src/lib/mrvDraw",
    "src/lib/mrvEdit",
    "src/lib/mrvFl",
    "src/lib/mrvFlU",
    "src/lib/mrvGL",
    "src/lib/mrvVk",
    "src/lib/mrvHDR",
    "src/lib/mrvHDRWidgets",
    "src/lib/mrvNetwork",
    "src/lib/mrvPanels",
    "src/lib/mrvPy",
    "src/lib/mrvUI",
]

CMAKE_DIRS = CPP_DIRS + [
    "cmake/",
    "cmake/nsis",
    "cmake/Modules",
    "src/src",
    "src/lib",
    "src/python/demos/fltk",
    "src/python/demos",
    "src/python/plug-ins",
    "src/bin",
]

BASH_DIRS = [
    ".",
    "bin",
    "bin/helpers",
    "bin/release",
    "bin/stats",
    "src/bin",
    "etc",
    "certificates",
    ".githooks"
]

def process_cpp_files():
    for cpp_dir in CPP_DIRS:
        print("Processing",cpp_dir)
        cpp_files = glob.glob( cpp_dir + "/*.cpp" )
        h_files = glob.glob( cpp_dir + "/*.h" )
        glsl_files = glob.glob( cpp_dir + "/*.glsl" )

        files = cpp_files + h_files + glsl_files

        for f in files:
            with open( f ) as ip:
                text = ip.read()
            with open( f + ".new", "w" ) as out:

                if not re.search( "Copyright", text ):
                    license = """// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

"""
                    print("Adding copyright to",f)
                    text = license + text

                out.write( text )

            shutil.move( f + ".new", f )


def process_cmake_files():
  for cmake_dir in CMAKE_DIRS:
      print("Processing",cmake_dir)
      aux_files = glob.glob( cmake_dir + "/*.cmake" )
      cmakelist_files = glob.glob( cmake_dir + "/CMakeLists.txt" )
      python_files = glob.glob( cmake_dir + "/*.py" )

      files = aux_files + cmakelist_files + python_files

      for f in files:
            with open( f, encoding='utf-8' ) as ip:
                text = ip.read()
            with open( f + ".new", "w", encoding='utf-8' ) as out:

                if not re.search( "Copyright", text ):
                    license = """# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

"""
                    print("Adding copyright to",f)
                    text = license + text

                out.write( text )

            shutil.move( f + ".new", f )
 


def process_bash_files():
    for bash_dir in BASH_DIRS:
        print("Processing",bash_dir)
        files = glob.glob( bash_dir + "/*.sh" )

        for f in files:
            with open( f ) as ip:
                text = ip.read()
                
            has_license = False
            if re.search( "Copyright", text ):
                has_license = True

            if has_license == False:
                with open( f + ".new", "w" ) as out:

                    license = """# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

"""
                    print("Adding copyright to",f)
                    
                    re.sub( "^#!.*sh", '', text )
                    shebang = "#!/usr/bin/env bash\n"
                    text = shebang + license + text

                    out.write( text )

                shutil.move( f + ".new", f )
                os.chmod(f, 0o755)


process_cpp_files()
process_cmake_files()
process_bash_files()

