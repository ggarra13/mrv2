// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

#include "mrvOptions/mrvStereo3DOptions.h"
#include "mrvOptions/mrvFilesPanelOptions.h"

#include "mrvDraw/Annotation.h"

namespace mrv
{
    using namespace tl;
    
    /**
     * \struct mrv::FilesModelItem
     * \brief This class models a media file item.
     *
     */
    struct FilesModelItem
    {
        tl::file::Path path;
        tl::file::Path audioPath;

        bool init = false;

        otime::TimeRange timeRange = time::invalidTimeRange;
        io::Info ioInfo;

        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Stop;
        timeline::Loop loop = timeline::Loop::Loop;
        otime::RationalTime currentTime = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;

        std::vector<std::string> videoLayers;
        size_t videoLayer = 0;

        float volume = 0.F;
        bool mute = false;
        double audioOffset = 0.0;

        std::string ocioIcs;
        
        std::vector<std::shared_ptr<draw::Annotation > > annotations;
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

        //! Create a new files model.
        static std::shared_ptr<FilesModel>
        create(const std::shared_ptr<system::Context>&);

        //! Observe the files.
        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        observeFiles() const;

        //! Observe the A file.
        std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > >
        observeA() const;

        //! Observe the A file index.
        std::shared_ptr<observer::IValue<int> > observeAIndex() const;

        //! Observe the B files.
        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        observeB() const;

        //! Observe the B file indexes.
        std::shared_ptr<observer::IList<int> > observeBIndexes() const;

        //! Observe the Stereo file.
        std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > >
        observeStereo() const;

        //! Observe the A file index.
        std::shared_ptr<observer::IValue<int> > observeStereoIndex() const;

        //! Observe the active files.
        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        observeActive() const;

        //! Add a file.
        void add(const std::shared_ptr<FilesModelItem>&);

        //! Replace a file at a certain index.
        void replace(const std::size_t, const std::shared_ptr<FilesModelItem>&);

        //! Close the current A file.
        void close();

        //! Close all the files.
        void closeAll();

        //! Set the A file.
        void setA(int index);

        //! Set the B files.
        void setB(int index, bool);

        //! Set the stereo file.
        void setStereo(int index);

        //! Toggle a B file.
        void toggleB(int index);

        //! Clear the B files.
        void clearB();

        //! Set the A file to the first file.
        void first();

        //! Set the A file to the list file.
        void last();

        //! Set the A file to the next file.
        void next();

        //! Set the A file to the previous file.
        void prev();

        //! Set the A file to the first file.
        void firstB();

        //! Set the A file to the list file.
        void lastB();

        //! Set the A file to the next file.
        void nextB();

        //! Set the A file to the previous file.
        void prevB();

        //! Observe the layers.
        std::shared_ptr<observer::IList<int> > observeLayers() const;

        //! Set a layer.
        void setLayer(const std::shared_ptr<FilesModelItem>&, int layer);

        //! Set the A file layer to the next layer.
        void nextLayer();

        //! Set the A file layer to the previous layer.
        void prevLayer();

        //! Observe the compare options.
        std::shared_ptr<observer::IValue<timeline::CompareOptions> >
        observeCompareOptions() const;

        //! Set the compare options.
        void setCompareOptions(const timeline::CompareOptions&);

        //! Observe the stereo3d options.
        std::shared_ptr<observer::IValue<Stereo3DOptions> >
        observeStereo3DOptions() const;

        //! Set the stereo 3d options.
        void setStereo3DOptions(const Stereo3DOptions&);

        //! Observe the stereo3d options.
        std::shared_ptr<observer::IValue<FilesPanelOptions> >
        observeFilesPanelOptions() const;

        //! Set the FilesPanel Options.
        void setFilesPanelOptions(const FilesPanelOptions&);

    private:
        int _index(const std::shared_ptr<FilesModelItem>&) const;
        std::vector<int> _bIndexes() const;
        std::vector<std::shared_ptr<FilesModelItem> > _getActive() const;
        std::vector<int> _getLayers() const;

        TLRENDER_PRIVATE();
    };

} // namespace mrv
