// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <FL/Fl_Window.H>

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

} // namespace tl

class ViewerUI;

namespace mrv
{
    using namespace tl;

    //! Open with separate audio dialog.
    class OpenSeparateAudioDialog : public Fl_Window
    {
    public:
        OpenSeparateAudioDialog(
            const std::shared_ptr<system::Context>&, ViewerUI*);
        ~OpenSeparateAudioDialog() override;

        const std::string videoFileName() const;
        const std::string audioFileName() const;

        void setVideoFileName(const std::string&);
        void setAudioFileName(const std::string&);

        void _browseVideoCallback(Fl_Widget*, void*);
        void _browseAudioCallback(Fl_Widget*, void*);

        bool exec();

        void make_exec_return(bool b)
        {
            m_exec = b;
            hide();
        }

    private:
        static void _browseVideoCallback_cb(Fl_Widget*, void*);
        static void _browseAudioCallback_cb(Fl_Widget*, void*);

        static void _ok_cb(Fl_Widget*, void*);
        static void _cancel_cb(Fl_Widget*, void*);

        bool m_exec = false;

    private:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
