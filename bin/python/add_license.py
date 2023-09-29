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

CPP_DIRS = [
    "mrv2/src",
    "mrv2/lib/mrvApp",
    "mrv2/lib/mrvCore",
    "mrv2/lib/mrvWidgets",
    "mrv2/lib/mrvDraw",
    "mrv2/lib/mrvEdit",
    "mrv2/lib/mrvFl",
    "mrv2/lib/mrvFlU",
    "mrv2/lib/mrvGL",
    "mrv2/lib/mrvNetwork",
    "mrv2/lib/mrvPanels",
    "mrv2/lib/mrvPy",
    "mrv2/lib/mrvUI",
]

CMAKE_DIRS = CPP_DIRS + [
    "cmake/",
    "cmake/nsis",
    "cmake/Modules",
    "mrv2/src",
    "mrv2/lib",
    "mrv2/python/demos",
    "mrv2/python/plug-ins",
    "mrv2/bin",
]

BASH_DIRS = [
    ".",
    "bin",
    "etc",
    "windows/envvars",
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
            with open( f ) as ip:
                text = ip.read()
            with open( f + ".new", "w" ) as out:

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
                if os.name == 'nt':
                    subprocess.run(['icacls', f, '/grant', '*S-1-1-0:(RX)'])
                else:
                    os.chmod(f, 0o755)


process_cpp_files()
process_cmake_files()
process_bash_files()

