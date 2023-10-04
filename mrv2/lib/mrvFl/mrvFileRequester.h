// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include <tlCore/Context.h>

#include <mrvCore/mrvString.h>

namespace mrv
{
    std::string file_single_requester(
        const char* title, const char* pattern, const char* startfile,
        const bool compact_files);

    const std::string file_save_single_requester(
        const char* title, const char* pattern, const char* startfile,
        const bool compact_images = true);

    std::string open_directory(const char* startfile);

    std::string open_session(const char* startfilex);

    /**
     * Opens a file requester to save an otio file
     *
     * @param startfile directory to start from
     *
     * @return file to save or empty string.
     */
    std::string save_otio(const char* startfile);

    /**
     * Opens a file requester to load an image
     *
     * @param startfile start filename (directory)
     *
     * @return opened filename(s)
     */
    std::vector<std::string>
    open_image_file(const char* startfile, const bool compact_files);

    // void attach_ocio_input_color_space( CMedia* img, ImageView* view );
    // void attach_ocio_display( CMedia* img, ImageView* view );
    // void attach_ocio_view( CMedia* img, ImageView* view );

    std::string open_python_file(const char* startfile);

    std::string save_python_file(const char* startfile);

    /**
     * Opens a file requester to load a lut file
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened subtitle file or empty
     */
    std::string open_lut_file(const char* startfile);

    /**
     * Opens a file requester to load a subtitle file
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened subtitle file or empty
     */
    std::string open_subtitle_file(const char* startfile);

    /**
     * Opens a file requester to load audio files
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened audio file or null
     */
    std::string open_audio_file(const char* startfile);

    std::string save_single_image(const char* startfile = nullptr);

    std::string save_movie_or_sequence_file(const char* startfile = nullptr);

    std::string save_pdf(const char* startdir = nullptr);

    std::string open_session_file(const char* startfile = nullptr);

    std::string save_session_file(const char* startfile = nullptr);

    std::string open_ocio_config(const char* startfile);

} // namespace mrv
