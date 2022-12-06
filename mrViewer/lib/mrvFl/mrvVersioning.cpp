
#include <boost/regex.hpp>

#include "mrvFl/mrvIO.h"
#include "mrViewer.h"

namespace
{
    const char* kModule = "version";
}

const boost::regex version_regex( ViewerUI* ui )
{
        
    boost::regex expr;
    std::string suffix;
    static std::string short_prefix = "_v";
    std::string prefix = short_prefix; // prefs->uiPrefsImageVersionPrefix->value();
    if ( prefix.empty() )
    {
        LOG_ERROR( _("Prefix cannot be an empty string.  Please type some regex to distinguish the version in the filename.  If unsure, use _v.") );
        return expr;
    }

    if ( prefix.size() < 5 )
    {
        short_prefix = prefix;
        LOG_INFO( _("Regex ") << prefix <<
                  (" replaced by complex regex.") );
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
  
