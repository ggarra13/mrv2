// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <limits>
#include <filesystem>

#include <tlCore/StringFormat.h>

namespace fs = std::filesystem;

#include "mrViewer.h"

#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvFile.h"

#include "mrvFl/mrvPathMapping.h"
#include "mrvFl/mrvVersioning.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "version";
}

namespace mrv
{

    const std::regex version_regex(const ViewerUI* ui, const bool verbose)
    {
        std::regex expr;
        std::string suffix;
        std::string prefix;
        static std::string short_prefix = "_v";
        std::string orig = ui->uiPrefs->uiPrefsVersionRegex->value();
        if (orig.empty())
            orig = short_prefix;

        if (orig.size() < 5)
        {
            prefix = "([\\w\\\\:/]*?[/\\._]*" + orig + ")(\\d+)([\\w\\d\\./]*)";
            if (verbose)
            {
                std::string msg = tl::string::Format(
                                      _("Regular expression created from {0}.  "
                                        "It is:\n{1}"))
                                      .arg(orig)
                                      .arg(prefix);
                LOG_INFO(msg);
            }
        }
        else
        {
            prefix = orig;
        }

        try
        {
            expr = prefix;
        }
        catch (const std::regex_error& e)
        {
            std::string msg =
                tl::string::Format(_("Regular expression error: {0}"))
                    .arg(e.what());
            LOG_ERROR(msg);
        }

        return expr;
    }

    std::string media_version(
        const ViewerUI* ui, const file::Path& path, int sum,
        const bool first_or_last)
    {
        const std::regex& expr = version_regex(ui, true);
        if (std::regex_match("", expr))
            return "";

        unsigned short tries = 0;
        bool found = false;
        int64_t start = std::numeric_limits<int>::max();
        std::string newfile, loadfile, suffix;
        unsigned max_tries = ui->uiPrefs->uiPrefsMaxImagesApart->value();

        std::string msg;
        std::string file = path.get();
        replace_path(file);

        while ((first_or_last || found == false) &&
               tries <= max_tries)
        {
            std::string::const_iterator tstart = file.begin();
            std::string::const_iterator tend = file.end();
            std::smatch what;
            std::regex_constants::match_flag_type flags = std::regex_constants::match_default;

            newfile.clear();
            try
            {
                unsigned iter = 1;
                while (std::regex_search(tstart, tend, what, expr, flags))
                {
                    std::string prefix = what[1];
                    std::string version = what[2];
                    suffix = what[3];

                    newfile += prefix;

                    if (!version.empty())
                    {
                        int padding = int(version.size());

                        // We don't allow negative versions
                        int num = std::stoi(version) + sum;
                        if (num < 0) num = 0;
                        
                        char buf[128];
                        snprintf(buf, 128, "%0*d", padding, num);
                        msg = tl::string::Format(
                            _("Iteration {0} will check version={1}"))
                              .arg(iter)
                              .arg(buf);
                        LOG_INFO(msg);
                        newfile += buf;
                    }

                    // Update iterator and flags for subsequent matches
                    tstart = what[3].first; // Move start position

                    // Ensure overlap if necessary
                    flags |= std::regex_constants::match_prev_avail; 
                    ++iter;
                }

                if (newfile.empty())
                {
                    LOG_ERROR(_("No versioning in this clip.  "
                                "Please create an image or directory "
                                "named with a versioning string."));

                    LOG_ERROR(_("Example:  gizmo_v003.0001.exr"));
                    return "";
                }

                newfile += suffix;

                if (file::isReadable(newfile))
                {
                    loadfile = newfile;
                    found = true;
                    if (!first_or_last)
                        break;
                }
                
                file = newfile;
            }
            catch (const std::regex_error& e)
            {
                std::string err = tl::string::Format(
                    _("Regular expression error: {0}")).arg(e.what());
                LOG_ERROR(err);
            }
            

            ++tries;
        }

        return loadfile;
    }

} // namespace mrv
