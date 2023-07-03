// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
namespace py = pybind11;

#include <FL/Fl_Menu.H>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvMenus.h"

#include "mrvWidgets/mrvPythonOutput.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvPy/PyStdErrOutRedirect.h"

#include "mrvApp/App.h"

namespace
{
    const char* kModule = "python";
    const std::string kPattern = ".py";
} // namespace

namespace mrv
{
    using namespace tl;

    void process_python_plugin(const std::string& file, py::module& plugin)
    {
        try
        {
            std::string moduleName = file.substr(0, file.size() - 3);

            py::module module = py::module::import(moduleName.c_str());

            // Get the __dict__ attribute of the user-defined module
            py::dict module_dict = module.attr("__dict__");

            py::object baseObj = plugin.attr("Plugin");

            for (auto item : module_dict)
            {
                std::string class_name = py::str(item.first);
                py::handle handle = item.second;

                if (!py::isinstance<py::class_<py::object>>(handle))
                    continue;

                py::object cls = py::reinterpret_borrow<py::object>(handle);

                // Check if cls is a subclass of baseObj
                if (!py::bool_(PyObject_IsSubclass(cls.ptr(), baseObj.ptr())))
                    continue;

                // Instantiate the class
                py::object pluginObj = module.attr(class_name.c_str())();

                bool isTrue = true;

                if (py::hasattr(pluginObj, "active"))
                {
                    py::object status = pluginObj.attr("active")();

                    // Convert the result to bool using py::cast
                    isTrue = py::cast<bool>(status);
                }

                if (!isTrue)
                    continue;

                if (py::hasattr(pluginObj, "menus"))
                {
                    py::dict menuDict = pluginObj.attr("menus")();

                    for (const auto& item : menuDict)
                    {
                        std::string menu = py::cast<std::string>(item.first);
                        py::handle method = item.second;
                        method.inc_ref();
                        pythonMenus.insert(menu, method);
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }
    }

    void discover_python_plugins(py::module m)
    {
        std::unordered_map<std::string, std::string> plugins;
        const std::vector<std::string>& paths = python_plugin_paths();
        for (const auto& path : paths)
        {
            for (const auto& entry : fs::directory_iterator(path))
            {
                if (entry.is_regular_file() &&
                    fs::path(entry).extension() == kPattern)
                {
                    const std::string& file =
                        entry.path().filename().generic_string();
                    if (plugins.find(file) != plugins.end())
                    {
                        std::string err =
                            string::Format(_("Duplicated Python plugin {0} in "
                                             "{1} and {2}."))
                                .arg(file)
                                .arg(path)
                                .arg(plugins[file]);
                        LOG_ERROR(err);
                        continue;
                    }
                    plugins[file] = path;
                }
            }
        }

        // Import the sys module
        py::module sys = py::module::import("sys");

        // Access the sys.path list
        py::list sysPath = sys.attr("path");

        for (const auto& path : paths)
        {
            // Add the additional directory to the sys.path list
            sysPath.attr("append")(path);
        }

        for (const auto& plugin : plugins)
        {
            process_python_plugin(plugin.first, m);
        }
    }

    void run_python_method_cb(Fl_Menu_* m, void* d)
    {
        ViewerUI* ui = App::ui;
        py::handle func = *(static_cast<py::handle*>(d));
        try
        {
            PyStdErrOutStreamRedirect pyRedirect;
            func();

            const std::string& out = pyRedirect.stdoutString();
            const std::string& err = pyRedirect.stderrString();

            if (!out.empty() || !err.empty())
            {
                if (!pythonPanel)
                    python_panel_cb(nullptr, ui);
                PythonOutput* output = PythonPanel::output();
                if (output)
                {
                    if (!out.empty())
                    {
                        output->info(out.c_str());
                    }
                    if (!err.empty())
                    {
                        output->error(out.c_str());
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }
    }

    class Plugin
    {
    public:
        Plugin(){};
        virtual ~Plugin(){};
        virtual bool active() const { return true; };
        virtual py::dict menus() const
        {
            throw std::runtime_error(
                _("Please override the menus method by returning a valid dict "
                  "of key menus, values methods."));
            py::dict m;
            return m;
        };
    };

} // namespace mrv

void mrv2_discover_python_plugins()
{
    py::module module = py::module::import("mrv2.plugin");
    mrv::discover_python_plugins(module);
}

void mrv2_python_plugins(pybind11::module& m)
{
    using namespace mrv;

    py::module plugin = m.def_submodule("plugin");
    plugin.doc() = _(R"PYTHON(
Plugin module.

Contains all classes related to python plugins.
)PYTHON");

    // Bind the Plugin base class
    py::class_<mrv::Plugin, std::shared_ptr<mrv::Plugin>>(plugin, "Plugin")
        .def(py::init<>())
        .def(
            "active", &mrv::Plugin::active,
            _("Whether a plugin is active or not.  If not overriden, the "
              "default is True."))
        .def("menus", &mrv::Plugin::menus, _(R"PYTHON(
Dictionary of menu entries with callbacks, like:

   def menus(self):
       menus = { "Nem Menu/Hello" : self.run }
       return menus

)PYTHON"))
        .doc() = _(R"PYTHON(
Base Plugin class.  
Must be overriden in a plugin Python file, like:

import mrv2
from mrv2 import timeline, plugin

class DemoPlugin(plugin.Plugin):
   """
   Define your own variables here.
   """
   def __init__(self):
       super().__init__()
       pass

   """
   Example method used for the callback.
   """
   def run(self):
       print("Hello from Python plugin")

   """
   Optional method to return whether the plug-in is active or not.
   """
   def active(self):
       return True

   """
   Dictionary of menu entries as keys with callbacks as values.
   """
   def menus(self):
      menus = { "New Menu/Hello" : self.run.
                "New Menu/Play/Forward" : timeline.playForward }
      return menus

)PYTHON");
}
