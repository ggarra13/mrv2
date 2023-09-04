// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvCore/mrvString.h>

#include <FL/Fl_Preferences.H>

class ViewerUI;

namespace mrv
{

    std::string open_directory(const char* startfile, ViewerUI* main);

    std::string open_session(const char* startfile, ViewerUI* main);

    /**
     * Opens a file requester to save an otio file
     *
     * @param startfile directory to start from
     *
     * @return file to save or empty string.
     */
    std::string save_otio(const char* startfile, ViewerUI* main);

    /**
     * Opens a file requester to load an image
     *
     * @param startfile start filename (directory)
     *
     * @return opened filename(s)
     */
    stringArray open_image_file(
        const char* startfile, const bool compact_files, ViewerUI* main);

    // void attach_ocio_input_color_space( CMedia* img, ImageView* view );
    // void attach_ocio_display( CMedia* img, ImageView* view );
    // void attach_ocio_view( CMedia* img, ImageView* view );

    std::string open_python_file(const char* startfile, ViewerUI* ui);

    std::string save_python_file(const char* startfile, ViewerUI* ui);

    /**
     * Opens a file requester to load a lut file
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened subtitle file or empty
     */
    std::string open_lut_file(const char* startfile, ViewerUI* main);

    /**
     * Opens a file requester to load a subtitle file
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened subtitle file or empty
     */
    std::string open_subtitle_file(const char* startfile, ViewerUI* main);

    /**
     * Opens a file requester to load audio files
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened audio file or null
     */
    std::string open_audio_file(const char* startfile, ViewerUI* main);

    std::string
    save_single_image(ViewerUI* ui, const char* startfile = nullptr);

    std::string
    save_movie_or_sequence_file(ViewerUI* ui, const char* startfile = nullptr);

    std::string save_pdf(ViewerUI* ui, const char* startdir = nullptr);

    std::string
    open_session_file(ViewerUI* ui, const char* startfile = nullptr);

    std::string
    save_session_file(ViewerUI* ui, const char* startfile = nullptr);

    std::string open_ocio_config(const char* startfile, ViewerUI* main);

    void load_hotkeys(ViewerUI* uiMain, std::string filename = "");
    void load_hotkeys(ViewerUI* uiMain, Fl_Preferences* prefs);
    void save_hotkeys(ViewerUI* uiMain, std::string filename = "");
    void save_hotkeys(Fl_Preferences& keys);

} // namespace mrv
