// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <map>
#include <string>
#include <vector>

#include <iostream>

#include <FL/Fl_Double_Window.H>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvIO.h"

#include "mrvWidgets/mrvBrowser.h"

#include "PathMappingUI.h"
#include "mrViewer.h"

namespace
{
    const char* kModule = "path";
}

namespace
{
    std::string add_slash(const std::string& in)
    {
        std::string out = in;
        int len = out.size() - 1;
        if (len < 0)
            return out;
        if (out[len] != '/' && out[len] != '\\')
            out += '/';
        return out;
    }
} // namespace

namespace mrv
{

    void add_path_mapping(mrv::Browser* b)
    {
        PathMappingUI map;
        Fl_Double_Window* w = map.uiMain;
        w->show();

        while (w->visible())
            Fl::check();

        std::string remote = map.RemotePath->value();
        std::string local = map.LocalPath->value();
        if (local.empty() || remote.empty())
            return;

        local = add_slash(local);
        remote = add_slash(remote);

        std::string mapping = remote + '\t' + local;
        b->add(mapping.c_str());
    }

    void change_path_mapping(mrv::Browser* b, int idx)
    {
        std::string line = b->text(idx);
        std::vector<std::string> splitArray;
        split(splitArray, line, '\t');

        std::string remote = splitArray[0];
        std::string local = splitArray[1];

        PathMappingUI map(remote, local);
        Fl_Double_Window* w = map.uiMain;
        w->show();

        while (w->visible())
            Fl::check();

        remote = map.RemotePath->value();
        local = map.LocalPath->value();
        if (local.empty() || remote.empty())
            return;

        local = add_slash(local);
        remote = add_slash(remote);

        std::string mapping = remote + '\t' + local;
        b->replace(idx, mapping.c_str());
    }

    void remove_path_mapping(mrv::Browser* b)
    {
        int idx = b->value();
        if (idx <= 1) // 1 for browser offset, 1 for title
            return;
        b->remove(idx);
    }

    std::map< std::string, std::string > path_mappings()
    {
        std::map< std::string, std::string> map;
        mrv::Browser* b = App::ui->uiPrefs->PathMappings;
        for (int i = 2; i <= b->size(); ++i)
        {
            std::string line = b->text(i);
            std::vector<std::string> splitArray;
            split(splitArray, line, '\t');
            map.insert(std::make_pair(splitArray[0], splitArray[1]));
        }
        return map;
    }

    bool replace_path(std::string& file)
    {
        if (is_readable(file))
            return true;

        const std::map< std::string, std::string >& map = path_mappings();
        for (const auto& item : map)
        {
            const std::string& remote = item.first;
            const std::string& local = item.second;
            std::string msg =
                tl::string::Format(_("Comparing {0} to prefix {1}"))
                    .arg(file)
                    .arg(remote);
            LOG_INFO(msg);
            if (file.substr(0, remote.size()) == remote)
            {
                std::string outFile = file;
                outFile.replace(0, remote.size(), local);

                if (is_readable(outFile))
                {
                    file = outFile;
                    msg =
                        tl::string::Format(_("Found a match.  File is now {0}"))
                            .arg(file);
                    LOG_INFO(msg);
                    return true;
                }
            }
        }
        return false;
    }
} // namespace mrv
