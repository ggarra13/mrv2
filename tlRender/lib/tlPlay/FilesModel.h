// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Path.h>
#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Files model item.
        struct FilesModelItem
        {
            file::Path path;
            file::Path audioPath;

            std::vector<std::string> videoLayers;
            size_t videoLayer = 0;
        };

        //! Files model.
        class FilesModel : public std::enable_shared_from_this<FilesModel>
        {
            TLRENDER_NON_COPYABLE(FilesModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            FilesModel();

        public:
            ~FilesModel();

            //! Create a new model.
            static std::shared_ptr<FilesModel>
            create(const std::shared_ptr<system::Context>&);

            //! Get the files.
            const std::vector<std::shared_ptr<FilesModelItem> >&
            getFiles() const;

            //! Observe the files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
            observeFiles() const;

            //! Get the "A" file.
            const std::shared_ptr<FilesModelItem>& getA() const;

            //! Observe the "A" file.
            std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > >
            observeA() const;

            //! Get the "A" file index.
            int getAIndex() const;

            //! Observe the "A" file index.
            std::shared_ptr<observer::IValue<int> > observeAIndex() const;

            //! Get the "B" files.
            const std::vector<std::shared_ptr<FilesModelItem> >& getB() const;

            //! Observe the "B" files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
            observeB() const;

            //! Get the "B" file indexes.
            const std::vector<int>& getBIndexes() const;

            //! Observe the "B" file indexes.
            std::shared_ptr<observer::IList<int> > observeBIndexes() const;

            //! Get the active files. The active files are the "A" file and
            //! "B" files.
            const std::vector<std::shared_ptr<FilesModelItem> >&
            getActive() const;

            //! Observe the active files. The active files are the "A" file
            //! and "B" files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
            observeActive() const;

            //! Add a file.
            void add(const std::shared_ptr<FilesModelItem>&);

            //! Close the current "A" file.
            void close();

            //! Close all the files.
            void closeAll();

            //! Reload all the active files.
            void reload();

            //! Set the "A" file.
            void setA(int index);

            //! Set the "B" files.
            void setB(int index, bool);

            //! Toggle a "B" file.
            void toggleB(int index);

            //! Clear the "B" files.
            void clearB();

            //! Set the "A" file to the first file.
            void first();

            //! Set the "A" file to the last file.
            void last();

            //! Set the "A" file to the next file.
            void next();

            //! Set the "A" file to the previous file.
            void prev();

            //! Set the "B" file to the first file.
            void firstB();

            //! Set the "B" file to the last file.
            void lastB();

            //! Set the "B" file to the next file.
            void nextB();

            //! Set the "B" file to the previous file.
            void prevB();

            //! Observe the layers.
            std::shared_ptr<observer::IList<int> > observeLayers() const;

            //! Set a layer.
            void setLayer(const std::shared_ptr<FilesModelItem>&, int layer);

            //! Set the "A" file to the next layer.
            void nextLayer();

            //! Set the "A" file to the previous layer.
            void prevLayer();

            //! Get the compare options.
            const timeline::CompareOptions& getCompareOptions() const;

            //! Observe the compare options.
            std::shared_ptr<observer::IValue<timeline::CompareOptions> >
            observeCompareOptions() const;

            //! Set the compare time mode.
            void setCompareTime(timeline::CompareTimeMode);

            //! Get the compare time mode.
            timeline::CompareTimeMode getCompareTime() const;

            //! Observe the compare time mode.
            std::shared_ptr<observer::IValue<timeline::CompareTimeMode> >
            observeCompareTime() const;

            //! Set the compare options.
            void setCompareOptions(const timeline::CompareOptions&);

        private:
            int _index(const std::shared_ptr<FilesModelItem>&) const;
            std::vector<int> _bIndexes() const;
            std::vector<std::shared_ptr<FilesModelItem> > _getActive() const;
            std::vector<int> _getLayers() const;

            TLRENDER_PRIVATE();
        };
    } // namespace play
} // namespace tl
