// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <mrvPlayApp/mrvOpenSeparateAudioDialog.h>

#include <tlTimeline/Timeline.h>

#include <tlCore/String.h>

#include <mrvCore/mrvI8N.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

#include <mrvFl/mrvFileRequester.h>



namespace mrv
{
    struct OpenSeparateAudioDialog::Private
    {
        std::weak_ptr<system::Context> context;
        ViewerUI*            ui = nullptr;
        Fl_Input* videoLineEdit = nullptr;
        Fl_Input* audioLineEdit = nullptr;
    };

    OpenSeparateAudioDialog::OpenSeparateAudioDialog(
        const std::shared_ptr<system::Context>& context,
        ViewerUI* ui ) :
        Fl_Window( 540, 280, _("Open with Audio") ),
        _p(new Private)
    {
        TLRENDER_P();

        p.context = context;
        p.ui      = ui;

        auto backdrop = new Fl_Group( 0, 0, w(), h() );
        backdrop->box( FL_FLAT_BOX );

        auto videoGroupBox = new Fl_Group( 20, 40, 520, 40 );
        p.videoLineEdit = new Fl_Input( 20, 40, 400, 40, _("Video")  );
        p.videoLineEdit->align( FL_ALIGN_TOP );
        auto videoBrowseButton = new Fl_Button( 440, 40, 80, 40, "@fileopen" );
        videoBrowseButton->callback( _browseVideoCallback_cb, this );
        videoGroupBox->end();

        auto audioGroupBox = new Fl_Group( 20, 100, 520, 40 );
        p.audioLineEdit = new Fl_Input( 20, 100, 400, 40, _("Audio") );
        p.audioLineEdit->align( FL_ALIGN_TOP );
        auto audioBrowseButton = new Fl_Button( 440, 100, 80, 40, "@fileopen");
        audioBrowseButton->callback( _browseAudioCallback_cb, this );
        audioGroupBox->end();

        auto button = new Fl_Button( 120, 160, 80, 40, _("OK") );
        button->callback( _ok_cb, this );

        button = new Fl_Button( 240, 160, 80, 40, _("Cancel") );
        button->callback( _cancel_cb, this );

        backdrop->end();

    }

    OpenSeparateAudioDialog::~OpenSeparateAudioDialog()
    {}

    const std::string OpenSeparateAudioDialog::videoFileName() const
    {
        return _p->videoLineEdit->value();
    }

    const std::string OpenSeparateAudioDialog::audioFileName() const
    {
        return _p->audioLineEdit->value();
    }

    void OpenSeparateAudioDialog::setVideoFileName(const std::string& value)
    {
        _p->videoLineEdit->value( value.c_str() );
    }

    void OpenSeparateAudioDialog::_browseVideoCallback( Fl_Widget* w, void* data )
    {
        TLRENDER_P();
        if (auto context = p.context.lock())
        {
            stringArray video = open_image_file("", true, p.ui);
            if ( video.empty() ) return;

            setVideoFileName( video[0] );
        }
    }

    void OpenSeparateAudioDialog::_browseAudioCallback( Fl_Widget* w, void* data )
    {
        TLRENDER_P();
        if (auto context = p.context.lock())
        {
            std::string audio = open_audio_file("", p.ui);
            setAudioFileName( audio );
        }
    }

    void OpenSeparateAudioDialog::setAudioFileName(const std::string& value)
    {
        _p->audioLineEdit->value( value.c_str() );
    }

    bool OpenSeparateAudioDialog::exec()
    {
        set_modal(); show();
        while ( visible() )
            Fl::check();
        return m_exec;
    }

    void OpenSeparateAudioDialog::_browseVideoCallback_cb( Fl_Widget* w, void* data )
    {
        OpenSeparateAudioDialog* self = static_cast<OpenSeparateAudioDialog*>( data );
        self->_browseVideoCallback( w, data );
    }

    void OpenSeparateAudioDialog::_browseAudioCallback_cb( Fl_Widget* w, void* data )
    {
        OpenSeparateAudioDialog* self = static_cast<OpenSeparateAudioDialog*>( data );
        self->_browseAudioCallback( w, data );
    }


    void OpenSeparateAudioDialog::_ok_cb( Fl_Widget* w, void* data )
    {
        OpenSeparateAudioDialog* self = static_cast<OpenSeparateAudioDialog*>( data );
        self->make_exec_return( true );
    }

    void OpenSeparateAudioDialog::_cancel_cb( Fl_Widget* w, void* data )
    {
        OpenSeparateAudioDialog* self = static_cast<OpenSeparateAudioDialog*>( data );
        self->make_exec_return( false );
    }


}
