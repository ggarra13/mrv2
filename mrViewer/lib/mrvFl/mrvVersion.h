/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvVersion.h
 * @author gga
 * @date   Wed Oct 25 01:45:43 2006
 *
 * @brief  Versioning information for mrViewer
 *
 *
 */

#ifndef mrvVersion_h
#define mrvVersion_h

#include <string>
#include "mrvFl/mrvBrowser.h"


namespace mrv
{

void ffmpeg_formats( mrv::Browser& b );
void ffmpeg_video_codecs( mrv::Browser& b );
void ffmpeg_audio_codecs( mrv::Browser& b  );
void ffmpeg_subtitle_codecs( mrv::Browser& b);
std::string ffmpeg_protocols();
void ffmpeg_motion_estimation_methods( mrv::Browser* b );


const char* version();
const char* build_date();

std::string about_message();


void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe);

std::string cpu_information();
std::string gpu_information( ViewerUI* uiMain );

} // namespace mrv


#endif // mrvVersion_h
