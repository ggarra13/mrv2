#!/usr/bin/env ruby
# encoding: utf-8
#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script searches .po files for untranslated strings and reports them.
#
# You must run it from the root of the mrv2 project.
#


require "fileutils"


if ARGV.size > 0
  langs = ARGV
else
  langs = [ 'ar', 'cs', 'de', 'en', 'es', 'fr', 'gr', 'it', 'ja',
            'ko', 'nl', 'pl', 'pt', 'ro', 'ru', 'sv', 'tr', 'zh' ]
end

translated = [ ]
for @lang in langs
  next if translated.any? @lang
  $stderr.puts "=================== Translate to #@lang ======================"
  in_msg_id = false
  msg = ''
  root = "mrv2/po"
  if not File.exist? "#{root}/#@lang.po"
    $stderr.puts "#{root}/#@lang.po does not exist."
    next
  end
  
  fp = File.open("#{root}/#@lang.po", "r", encoding: "utf-8")
  lines = fp.readlines
  fp.close

  @count = 0 # header

  while @count < 14
    @count += 1
  end

  fuzzy = false
  @msgid = ''
  num_lines = lines.size-1
  while @count < num_lines
    line = lines[@count]
    line2 = lines[@count+1]
    if in_msg_id
      msgstr = line =~ /^msgstr\s+"/
      line =~ /"(.*)"$/
      match = $1

      text2 = line2 =~ /^"(.*)"$/
      match2 = $1

      if match
        @msgid << match if not fuzzy
      end


      if msgstr
        if (match == '' or fuzzy) and not text2
          puts "Missing translate for #@msgid"
        end
        @msgid = ''
        in_msg_id = fuzzy = false
        @count += 1
        next
      end
      @count += 1
      next
    else
      if line =~ /fuzzy/
        fuzzy = true
        line.sub!( /fuzzy/, '' )
        @msgid = ''
      end
    end
    msgid = line =~ /^msgid "(.*)"/
    if msgid
      @msgid = $1
      in_msg_id = true
    end
    @count += 1
  end
end
