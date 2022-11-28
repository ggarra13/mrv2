#!/usr/bin/env ruby

require "fileutils"


DIRS = [ "../mrViewer/src",
         "../mrViewer/lib/mrvCore",
         "../mrViewer/lib/mrvWidgets",
         "../mrViewer/lib/mrvDraw",
         "../mrViewer/lib/mrvGL",
         "../mrViewer/lib/mrvFl",
         "../mrViewer/lib/mrvApp",
       ]

for dir in DIRS
  puts "Processing #{dir}"
  cpp_files = Dir.glob( dir + "/*.cpp" )
  h_files = Dir.glob( dir + "/*.h" )

  files = cpp_files + h_files
  
  for file in files
    text = File.readlines(file).join()
    out  = File.open( file + ".new", "w" )

    if text !~ /Copyright/
      license = "// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
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
