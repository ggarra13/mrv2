// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrv22)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <locale.h>

#include <FL/fl_utf8.h>

#include <Iex.h>

#include "mrvCore/mrvHome.h"

#include "mrvGLUtil.h"

namespace mrv
{
    

    std::string readGLShader( const std::string& filename )
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
