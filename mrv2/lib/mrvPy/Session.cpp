// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <map>
#include <string>

#include "mrvFl/mrvSession.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrViewer.h"

namespace mrv
{
    namespace session
    {

        /**
         * \brief Save a session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        bool save()
        {
            const std::string& file = session::current();
            if (file.empty())
            {
                throw std::runtime_error(
                    _("No session name established, cannot save."));
                return false;
            }
            return session::save(file);
        }

        /**
         * \brief Save a session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        bool saveAs(std::string file)
        {
            if (file.substr(file.size() - 6, file.size()) != ".mrv2s")
            {
                file += ".mrv2s";
            }
            return session::save(file);
        }

    } // namespace session
} // namespace mrv

/**
 * \cond
 *
 */
void mrv2_session(py::module& m)
{
    py::module session = m.def_submodule("session");
    session.doc() = _(R"PYTHON(
Session module.

Used to manage everything related to sessions.
)PYTHON");

    //
    // Session commands
    //
    session.def(
        "metadata", py::overload_cast<>(&mrv::session::metadata),
        _("Returns the current session metadata."));

    session.def(
        "metadata",
        py::overload_cast<const std::string&>(&mrv::session::metadata),
        _("Returns the current metadata for a key."), py::arg("key"));

    session.def(
        "setMetadata",
        py::overload_cast<const std::map<std::string, std::string>&>(
            &mrv::session::setMetadata),
        _("Sets all the session metadata for a session."));

    session.def(
        "setMetadata",
        py::overload_cast<const std::string&, const std::string&>(
            &mrv::session::setMetadata),
        _("Sets the session metadata for a key."), py::arg("key"),
        py::arg("value"));

    session.def(
        "clearMetadata", &mrv::session::clearMetadata,
        _("Clears the current session metadata."));

    session.def(
        "current", &mrv::session::current, _("Returns current session file."));

    session.def(
        "setCurrent", &mrv::session::setCurrent,
        _("Sets the current session file."), py::arg("file"));

    session.def(
        "load", &mrv::session::load, _("Open a session file."),
        py::arg("file"));

    session.def(
        "save", py::overload_cast<>(&mrv::session::save),
        _("Save a session file."));

    session.def(
        "save", py::overload_cast<std::string>(&mrv::session::saveAs),
        _("Save a session file."), py::arg("file"));
}
/**
 * \endcond
 *
 */
