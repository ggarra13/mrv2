#pragma once

#include <mrvFl/mrvTimeObject.h>

#include <any/any.hpp>
#include <vector>
#include <memory>

#ifdef LINB_ANY_HPP
#  define std_any linb::any
#  define std_any_cast linb::any_cast
#  define std_any_empty(x) x.empty()
#else
#  define std_any std::any
#  define std_any_cast std::any_cast
#  define std_any_empty(x) x.has_value()
#endif

namespace mrv
{
    //! Settings object.
    class SettingsObject 
    {

    public:
        SettingsObject( TimeObject* );

        ~SettingsObject();
        
        //! Get the list of keys in settings.
        const std::vector<std::string> keys() const;

        //! Get a settings value.
        std_any value(const std::string&);

        //! Get the list of recent files.
        const std::vector<std::string>& recentFiles() const;

        //! Get whether tooltips are enabled.
        bool hasToolTipsEnabled() const;

    public:  // Q_SLOTS
        //! Set a settings value.
        void setValue(const std::string&, const std_any&);

        //! Set a default settings value.
        void setDefaultValue(const std::string&, const std_any&);

        //! Reset the settings to defaults.
        void reset();

        //! Add a recent file.
        void addRecentFile(const std::string&);

    private:

        TLRENDER_PRIVATE();
    };
}
