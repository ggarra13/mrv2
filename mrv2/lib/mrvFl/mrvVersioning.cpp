// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <limits>
#include <filesystem>

#include <tlCore/StringFormat.h>

namespace fs = std::filesystem;

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
            prefix = "([\\w:/]*?[/\\._]*" + orig + ")(\\d+)([%\\w\\d\\./]*)";
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
        short add = sum;
        const std::regex& expr = version_regex(ui, true);
        if (std::regex_match("", expr))
            return "";

        unsigned short tries = 0;
        int64_t start = std::numeric_limits<int>::min();
        int64_t end = std::numeric_limits<int>::min();
        std::string newfile, loadfile, suffix;
        unsigned max_tries = ui->uiPrefs->uiPrefsMaxImagesApart->value();
        while ((first_or_last || start == std::numeric_limits<int>::min()) &&
               tries <= max_tries)
        {
            std::string file = path.get();
            std::string::const_iterator tstart, tend;
            tstart = file.begin();
            tend = file.end();
            std::match_results<std::string::const_iterator> what;
            std::regex_constants::match_flag_type flags =
                std::regex_constants::match_default;
            newfile.clear();
            try
            {
                unsigned iter = 1;
                LOG_INFO("====================================================="
                         "=======================");
                while (std::regex_search(tstart, tend, what, expr, flags))
                {
                    std::string prefix = what[1];
                    std::string number = what[2];
                    suffix = what[3];

                    std::string msg = tl::string::Format(
                                          _("Iteration {0} matched prefix={1}"))
                                          .arg(iter)
                                          .arg(prefix);
                    LOG_INFO(msg);
                    msg = tl::string::Format(
                              _("Iteration {0} matched version={1}"))
                              .arg(iter)
                              .arg(number);
                    LOG_INFO(msg);
                    msg = tl::string::Format(
                              _("Iteration {0} matched suffix={1}"))
                              .arg(iter)
                              .arg(suffix);
                    LOG_INFO(msg);
                    LOG_INFO("-------------------------------------------------"
                             "---------------------------");

                    newfile += prefix;

                    if (!number.empty())
                    {
                        int padding = int(number.size());
                        int num = atoi(number.c_str());
                        char buf[128];
                        snprintf(buf, 128, "%0*d", padding, num + sum);
                        msg = tl::string::Format(
                                  _("Iteration {0} will check version={1}"))
                                  .arg(iter)
                                  .arg(buf);
                        LOG_INFO(msg);
                        newfile += buf;
                    }

                    tstart = what[3].first;
                    flags |= std::regex_constants::match_prev_avail;
                    // flags |= std::regex_constants::match_not_bob;
                    ++iter;
                }
            }
            catch (const std::regex_error& e)
            {
                std::string msg =
                    tl::string::Format(_("Regular expression error: {0}"))
                        .arg(e.what());
                LOG_ERROR(msg);
            }

            if (newfile.empty())
            {
                LOG_ERROR(_("No versioning in this clip.  "
                            "Please create an image or directory named with "
                            "a versioning string."));

                LOG_ERROR(_("Example:  gizmo_v003.0001.exr"));
                return "";
            }

            newfile += suffix;

            if (mrv::is_valid_sequence(newfile.c_str()))
            {
                mrv::get_sequence_limits(start, end, newfile, false);
                if (start != std::numeric_limits<int64_t>::min())
                {
                    char fmt[1024], buf[1024];
                    snprintf(fmt, 1024, "%s", newfile.c_str());
                    snprintf(buf, 1024, fmt, start);
                    if (fs::exists(buf))
                    {
                        loadfile = buf;
                    }
                }
            }
            else
            {
                std::string ext = newfile;
                size_t p = ext.rfind('.');
                if (p != std::string::npos)
                {
                    ext = ext.substr(p, ext.size());
                }
                std::transform(
                    ext.begin(), ext.end(), ext.begin(), (int (*)(int))tolower);

                if (mrv::is_valid_movie(ext.c_str()))
                {
                    if (fs::exists(newfile))
                    {
                        loadfile = newfile;
                        start = 1;
                        if (!first_or_last)
                            break;
                    }
                }
            }

            ++tries;
            sum += add;
        }

        return loadfile;
    }

} // namespace mrv
