#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

@options = { :verbose => false }
OptionParser.new do |opts|
  opts.banner = "Usage: tools/rename.rb [@options] string replacement files..."

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!


string  = ARGV.shift
replace = ARGV.shift
files   = Dir.glob(ARGV)

for file in files
  text = File.read(file, :encoding => 'utf-8' )
  text.gsub!( /#{string}/, replace )
  w = File.open( "#{file}", 'w' )
  w.puts text
  w.close
end
