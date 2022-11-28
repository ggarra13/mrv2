#!/usr/bin/env ruby

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
    puts "\t#{file}"
    text = File.readlines(file)
    out  = File.open( file, "w" )
    license = "// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

"
    text = license + text.join()
    out.puts text
    out.close
  end
end
