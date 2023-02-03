// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <errno.h>   // errno
#include <cstring>  // strerror
#include <locale.h>  // setlocale


#include <FL/fl_utf8.h>

#include <Iex.h>

#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvUtil.h"
#include "mrvFl/mrvIO.h"

namespace
{
  const char* kModule = "util";
}

namespace mrv
{
    Fl_SVG_Image* load_svg( const std::string& svg_name )
    {
        std::string svg_root = rootpath();;
        svg_root += "/icons/";
    
        std::string file = svg_root + svg_name;
        Fl_SVG_Image* svg = new Fl_SVG_Image( file.c_str() );
        if ( !svg ) return nullptr;
      
        switch (svg->fail()) {
        case Fl_Image::ERR_FILE_ACCESS:
            // File couldn't load? show path + os error to user
            LOG_ERROR( file << ": " << strerror(errno) );
            return nullptr;
        case Fl_Image::ERR_FORMAT:
            // Parsing error
            LOG_ERROR( file << ": couldn't decode image" );
            return nullptr;
        }
        return svg;
    }

    
    std::string readShaderSource( const std::string& filename )
    {
        std::string shader = shaderpath() + filename;
        
        const char* oldloc= setlocale( LC_NUMERIC, "C" );
        FILE* f = fl_fopen( shader.c_str(), "rb" );
        if (!f)
        {
            THROW_ERRNO ("Can't load shader file '" << shader << "' (%T)" );
        }

        fseek( f, 0, SEEK_END );
        size_t len = ftell (f);
        fseek( f, 0, SEEK_SET );

        char* code = new char[len + 1];

        size_t read = fread( code, 1, len, f );
        if (read != len)
        {
            fclose(f);
            THROW (Iex::BaseExc, "Expected " << len << " bytes in fragment " <<
                   "shader file " << filename << ", only got " << read);
            
        }

        code[read] = '\0';    // null-terminate
        fclose(f);
    
        setlocale( LC_NUMERIC, oldloc );

        std::string result = code;
        delete [] code;
        
        return result;

    }
}
