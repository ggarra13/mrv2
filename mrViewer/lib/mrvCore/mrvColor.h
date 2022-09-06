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
 * @file   mrvColor.h
 * @author
 * @date   Fri Oct 13 10:11:05 2006
 *
 * @brief  Some fltk routines to deal with Fl_colors more easily.
 *
 *
 */

#ifndef mrvColor_h
#define mrvColor_h


#include "mrvCore/mrvImagePixel.h"

namespace mrv
{

  enum BrightnessType {
    kAsLuminance,
    kAsLumma,
    kAsLightness,
  };

  float calculate_brightness( const mrv::ImagePixel& rgba,
                              const mrv::BrightnessType type );

}


#endif // mrvColor_h
