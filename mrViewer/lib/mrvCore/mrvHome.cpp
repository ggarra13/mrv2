// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include <boost/filesystem.hpp>

#include "mrvHome.h"

#if defined(_WIN32) && !defined(_WIN64_)
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <pwd.h>
#endif

namespace fs = boost::filesystem;

namespace mrv
{

std::string sgetenv( const char* const n )
{
   if ( fl_getenv( n ) )
      return fl_getenv( n );
   else
      return std::string();
}

std::string homepath()
{
   std::string path;

#ifdef _WIN32
   char* e = NULL;
   if ( (e = fl_getenv("HOME")) )
   {
       path = e;
       size_t pos = path.rfind( "Documents" );
       if ( pos != std::string::npos )
       {
           path = path.replace( pos, path.size(), "" );
       }
       if ( fs::is_directory( path ) )
           return path;
   }
   if ( (e = fl_getenv("USERPROFILE")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   if ( (e = fl_getenv("HOMEDRIVE")) )
   {
       path = e;
       path += sgetenv("HOMEPATH");
       path += "/" + sgetenv("USERNAME");
       if ( fs::is_directory( path ) )
           return path;
   }
#else
   char* e = NULL;
   if ( (e = fl_getenv("HOME")) )
   {
       path = e;
       size_t pos = path.rfind( "Documents" );
       if ( pos != std::string::npos )
       {
           path = path.replace( pos, path.size(), "" );
       }
       if ( fs::is_directory( path ) )
           return path;
   }
   else
   {
     e = getpwuid( getuid() )->pw_dir;
     if ( e ) {
       path = e;
       return path;
     }
   }
#endif

   if ( (e = fl_getenv("TMP")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   if ( (e = fl_getenv("TEMP")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   path = "/usr/tmp";
   return path;
}

std::string studiopath()
{
    const char* c = fl_getenv( "STUDIOPATH" );
    if (!c) return "";
    std::string r = c;
    if ( r.substr( r.size() - 1, r.size() ) != "/" )
        r += "/";
    return r;
}

std::string prefspath()
{
    std::string studio = mrv::studiopath();
    if ( fs::is_directory( studio )  )
    {
        return studio;
    }
    std::string lockfile = mrv::homepath();
    lockfile += "/.filmaura/";
    return lockfile;
}

std::string lockfile()
{
    std::string lockfile = mrv::homepath();
    lockfile += "/.filmaura/mrViewer.lock.prefs";
    return lockfile;
}


}
