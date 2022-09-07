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
 * @file   mrvRoot.h
 * @author gga
 * @date   Fri Jan 11 02:04:55 2008
 *
 * @brief  sets the MRV_ROOT variable if not defined by trying to find
 *         the executable's root directory.
 *
 */

namespace mrv {

  void  set_root_path( const int argc = 0, char** argv = NULL );

}
