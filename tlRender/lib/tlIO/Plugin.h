// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/FileIO.h>
#include <tlCore/Path.h>

#include <future>
#include <set>


namespace tl
{
    namespace log
    {
        class System;
    }

    namespace io
    {
        //! Base class for readers and writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            TLRENDER_NON_COPYABLE(IIO);

        protected:
            void _init(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<log::System>&);

            IIO();

        public:
            virtual ~IIO() = 0;

            //! Get the path.
            const file::Path& getPath() const;

            //! Get the number of objects currenty instantiated.
            static size_t getObjectCount();

        protected:
            file::Path _path;
            io::Options _options;
            std::weak_ptr<log::System> _logSystem;
        };

        //! Base class for I/O plugins.
        class IIOPlugin : public std::enable_shared_from_this<IIOPlugin>
        {
            TLRENDER_NON_COPYABLE(IIOPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& exts,
                const std::shared_ptr<log::System>&);

            IIOPlugin();

        public:
            virtual ~IIOPlugin() = 0;

            //! Get the plugin name.
            const std::string& getPluginName() const;

            //! Get the plugin information.
            virtual std::string getPluginInfo(
                const io::Options& = io::Options()) const;

            //! Get the supported file extensions.
            std::set<std::string> getExts(int types =
                                          static_cast<int>(FileType::Movie) |
                                          static_cast<int>(FileType::Audio) |
                                          static_cast<int>(FileType::Sequence)) const;

        protected:
            std::weak_ptr<log::System> _logSystem;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
