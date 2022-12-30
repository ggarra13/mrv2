// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "mrViewer.h"

#include "mrvCore/mrvSequence.h"

#include "mrvFl/mrvVersioning.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "version";
}

namespace mrv
{

    const boost::regex version_regex( const ViewerUI* ui )
    {

        boost::regex expr;
        std::string suffix;
        static std::string short_prefix = "_v";
        std::string prefix = ui->uiPrefs->uiPrefsVersionRegex->value();
        if ( prefix.empty() ) prefix = short_prefix;

        if ( prefix.size() < 5 )
        {
            prefix = "([\\w:/]*?[/\\._]*" + prefix +
                     ")(\\d+)([%\\w\\d\\./]*)";
        }

        try
        {
            expr = prefix;
        }
        catch ( const boost::regex_error& e )
        {
            LOG_ERROR( _("Regular expression error: ") << e.what()  );
        }

        return expr;
    }

    std::string media_version( const ViewerUI* ui, const file::Path& path,
                               int sum, const bool first_or_last )
    {
        short add = sum;
        const boost::regex& expr = version_regex( ui );
        if ( expr.empty() ) return "";
        
        unsigned short tries = 0;
        int64_t start = AV_NOPTS_VALUE;
        int64_t end   = AV_NOPTS_VALUE;
        std::string newfile, loadfile, suffix;
        unsigned max_tries = ui->uiPrefs->uiPrefsMaxImagesApart->value();
        while ( (first_or_last || start == AV_NOPTS_VALUE ) &&
                tries <= max_tries )
        {
            std::string file = path.get();
            std::string::const_iterator tstart, tend;
            tstart = file.begin();
            tend = file.end();
            boost::match_results<std::string::const_iterator> what;
            boost::match_flag_type flags = boost::match_default;
            newfile.clear();
            try
            {
                unsigned iter = 1;
                LOG_INFO( "============================================================================" );
                while ( boost::regex_search( tstart, tend, what, expr, flags ) )
                {
                    std::string prefix = what[1];
                    std::string number = what[2];
                    suffix = what[3];

                    LOG_INFO( _("Iteration ") << iter
                              << _(" Matched prefix=") << prefix );
                    LOG_INFO( _("Iteration ") << iter
                              << _(" Matched number=") << number );
                    LOG_INFO( _("Iteration ") << iter
                              << _(" Matched suffix=") << suffix );
                    LOG_INFO( "----------------------------------------------------------------------------" );

                    newfile += prefix;

                    if ( !number.empty() )
                    {
                        int padding = int( number.size() );
                        int num = atoi( number.c_str() );
                        char buf[128];
                        sprintf( buf, "%0*d", padding, num + sum );
                        LOG_INFO( _("Iteration ") << iter
                                  << _(" will check version=") << buf );
                        newfile += buf;
                    }

                    tstart = what[3].first;
                    flags |= boost::match_prev_avail;
                    flags |= boost::match_not_bob;
                    ++iter;
                }
            }
            catch ( const boost::regex_error& e )
            {
                LOG_ERROR( _("Regular expression error: ") << e.what()  );
            }

            if ( newfile.empty() )
            {
                LOG_ERROR( _("No versioning in this clip.  "
                             "Please create an image or directory named with "
                             "a versioning string." ) );

                LOG_ERROR( _("Example:  gizmo_v003.0001.exr") );
                return "";
            }

            newfile += suffix;

            if ( mrv::is_valid_sequence( newfile.c_str() ) )
            {
                mrv::get_sequence_limits( start, end, newfile, false );
                if ( start != AV_NOPTS_VALUE ) {
                    char fmt[1024], buf[1024];
                    sprintf( fmt, "%s", newfile.c_str() );
                    sprintf( buf, fmt, start );
                    if ( fs::exists( buf ) )
                    {
                        loadfile = buf;
                    }
                }
            }
            else
            {
                std::string ext = newfile;
                size_t p = ext.rfind( '.' );
                if ( p != std::string::npos )
                {
                    ext = ext.substr( p, ext.size() );
                }
                std::transform( ext.begin(), ext.end(), ext.begin(),
                                (int(*)(int)) tolower );

                if ( mrv::is_valid_movie( ext.c_str() ) )
                {
                    if ( fs::exists( newfile ) )
                    {
                        loadfile = newfile;
                        start = 1;
                        if ( !first_or_last ) break;
                    }
                }
            }

            ++tries;
            sum += add;
        }
        
        return loadfile;
    }

}
