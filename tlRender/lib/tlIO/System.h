// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/Write.h>

#include <tlCore/ISystem.h>

namespace tl
{
    namespace io
    {
        //! Read system.
        class ReadSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(ReadSystem);

        protected:
            ReadSystem(const std::shared_ptr<system::Context>&);

        public:
            virtual ~ReadSystem();

            //! Create a new system.
            static std::shared_ptr<ReadSystem> create(const std::shared_ptr<system::Context>&);

            //! Get the list of plugins.
            const std::vector<std::shared_ptr<IReadPlugin> >& getPlugins() const;

            //! Add a plugin.
            void addPlugin(const std::shared_ptr<IReadPlugin>&);

            //! Remove a plugin.
            void removePlugin(const std::shared_ptr<IReadPlugin>&);

            //! Get a plugin.
            template<typename T>
            std::shared_ptr<T> getPlugin() const;

            //! Get a plugin for the given path.
            std::shared_ptr<IReadPlugin> getPlugin(const file::Path&) const;

            //! Get the plugin names.
            const std::vector<std::string>& getNames() const;

            //! Get the supported file extensions.
            std::set<std::string>
            getExts(int types =
                    static_cast<int>(FileType::Movie) |
                    static_cast<int>(FileType::Sequence)) const;

            //! Get the file type for the given extension.
            FileType getFileType(const std::string&) const;

            //! Create a video reader for the given path, or null when the
            //! format has no video or is decoded rather than read.
            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&,
                const io::Options& = io::Options());

            //! Create a video reader for the given path and memory locations.
            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options());

            //! Create an audio reader for the given path, or null when the
            //! format has no audio.
            std::shared_ptr<io::IAudioRead> audioRead(
                const file::Path&,
                const io::Options& = io::Options());

            //! Create an audio reader for the given path and memory locations.
            std::shared_ptr<io::IAudioRead> audioRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options());

        private:
            std::vector<std::shared_ptr<IReadPlugin> > _plugins;

            TLRENDER_PRIVATE();
        };

        //! Write system.
        class WriteSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(WriteSystem);

        protected:
            WriteSystem(const std::shared_ptr<system::Context>&);

        public:
            virtual ~WriteSystem();

            //! Create a new system.
            static std::shared_ptr<WriteSystem> create(const std::shared_ptr<system::Context>&);

            //! Get the list of plugins.
            const std::vector<std::shared_ptr<IWritePlugin> >& getPlugins() const;

            //! Add a plugin.
            void addPlugin(const std::shared_ptr<IWritePlugin>&);

            //! Remove a plugin.
            void removePlugin(const std::shared_ptr<IWritePlugin>&);

            //! Get a plugin.
            template<typename T>
            std::shared_ptr<T> getPlugin() const;

            //! Get a plugin for the given path.
            std::shared_ptr<IWritePlugin> getPlugin(const file::Path&) const;

            //! Get the plugin names.
            const std::vector<std::string>& getNames() const;

            //! Get the supported file extensions.
            std::set<std::string> getExts(int types =
                                          static_cast<int>(FileType::Movie) |
                                          static_cast<int>(FileType::Sequence)) const;

            //! Get the file type for the given extension.
            FileType getFileType(const std::string&) const;

            //! Create a writer for the given path.
            std::shared_ptr<IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options());

        private:
            std::vector<std::shared_ptr<IWritePlugin> > _plugins;

            TLRENDER_PRIVATE();
        };
    }
}

#include <tlIO/SystemInline.h>
