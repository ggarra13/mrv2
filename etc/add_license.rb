#!/usr/bin/env ruby

require "fileutils"


CPP_DIRS = [ "../mrv2/src",
             "../mrv2/lib/mrvCore",
             "../mrv2/lib/mrvWidgets",
             "../mrv2/lib/mrvDraw",
             "../mrv2/lib/mrvGL",
             "../mrv2/lib/mrvFl",
             "../mrv2/lib/mrvApp",
             "../mrv2/shaders/gl",
           ]

CMAKE_DIRS = CPP_DIRS +
             [
               "../cmake/",
               "../cmake/Modules",
             ]

def process_cpp_files
  for dir in CPP_DIRS
    puts "Processing #{dir}"
    cpp_files = Dir.glob( dir + "/*.cpp" )
    h_files = Dir.glob( dir + "/*.h" )
    glsl_files = Dir.glob( dir + "/*.glsl" )

    files = cpp_files + h_files + glsl_files

    for file in files
      text = File.readlines(file).join()
      out  = File.open( file + ".new", "w" )

      if text !~ /Copyright/
        license = "// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrv22)
// Copyright Contributors to the mrv2 Project. All rights reserved.

"
        puts "\tAdding copyright to #{file}"
        text = license + text
      end

      out.puts text
      out.close

      FileUtils.mv( file + ".new", file )
    end
  end
end


def process_cmake_files
  for dir in CMAKE_DIRS
    puts "Processing #{dir}"
    aix_files = Dir.glob( dir + "/*.cmake" )
    cmakelist_files = Dir.glob( dir + "/CMakeLists.txt" )

    files = aix_files + cmakelist_files

    for file in files
      text = File.readlines(file).join()
      out  = File.open( file + ".new", "w" )

      if text !~ /Copyright/
        license = "# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

"
        puts "\tAdding copyright to #{file}"
        text = license + text
      end

      out.puts text
      out.close

      FileUtils.mv( file + ".new", file )
    end
  end
end


process_cpp_files
process_cmake_files
