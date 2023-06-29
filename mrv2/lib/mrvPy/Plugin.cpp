// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <FL/Fl_Menu.H>

#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvMenus.h"

namespace
{
    const char* kModule = "python";
    const std::string kPattern = "*.py";
} // namespace

namespace mrv
{
    void process_python_plugin(const std::string file, const std::string path)
    {
        std::cerr << "Processing " << file << " from " << path << std::endl;
    }

    void discover_python_plugins()
    {
        std::unordered_map<std::string, std::string> plugins;
        const std::vector<std::string>& paths = python_plugin_paths();
        for (const auto& path : paths)
        {
            std::cerr << "Scanning path... " << path << std::endl;
            for (const auto& entry : fs::directory_iterator(path))
            {
                if (entry.is_regular_file() &&
                    fs::path(entry).extension() == kPattern)
                {
                    const std::string file = entry.path().filename();
                    std::cout << "\t" << file << std::endl;
                    if (plugins.find(file) != plugins.end())
                    {
                        std::cerr << "Duplicated Python plugin " << file
                                  << " in " << path << " and " << plugins[file]
                                  << std::endl;
                        continue;
                    }
                    plugins[file] = path;
                }
            }
        }

        for (const auto& plugin : plugins)
        {
            process_python_plugin(plugin.first, plugin.second);
        }
    }

    void run_python_method_cb(Fl_Menu_* m, void* d)
    {
        py::object func = *(static_cast<py::object*>(d));
        try
        {
            func();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }
    }

} // namespace mrv

void mrv2_python_plugins(pybind11::module& m)
{
    using namespace mrv;

    std::cout << "Looking for python plugins..." << std::endl;

    discover_python_plugins();
}
