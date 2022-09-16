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
 * @file   Sequence.h
 * @author
 * @date   Thu Oct 12 20:09:31 2006
 *
 * @brief  Routines used to parse image sequences
 *
 *
 */
#pragma once

#include <string>
#include <vector>

#include <mrvCore/mrvString.h>

inline std::string uncomment_slashes( std::string path )
{
    size_t found = 0;
    while( (found = path.find("\\/", found)) != std::string::npos )
    {
        std::string part2 = path.substr( found + 2, path.size() );
        path = path.substr(0, found) + part2;
        found += 2;
    }

    return path;
}

inline std::string comment_slashes( std::string path )
{
    size_t found = 0, next = 0;
    while( (found = path.find('/', next)) != std::string::npos )
    {
        path.insert(found, "\\");
        next = found+3;
    }

    return path;
}


namespace mrv
{

extern const int64_t kMinFrame;
extern const int64_t kMaxFrame;


struct Sequence
{
    std::string root;
    std::string number;
    std::string view;
    std::string ext;
};

typedef std::vector< Sequence > Sequences;

struct SequenceSort
{
    // true if a < b, else false
    bool operator()( const Sequence& a, const Sequence& b ) const
    {
        if ( a.root < b.root )  return true;
        else if ( a.root > b.root ) return false;

        if ( a.ext < b.ext ) return true;
        else if ( a.ext > b.ext ) return false;

        if ( a.view < b.view ) return true;
        else if ( a.view > b.view ) return false;

        // size_t as = a.number.size();
        // size_t bs = b.number.size();
        // if ( as < bs ) return true;
        // else if ( as > bs ) return false;

        if ( atoi( a.number.c_str() ) < atoi( b.number.c_str() ) )
            return true;
        return false;
    }
};



/**
 * Given a filename with %hex characters, return string in ascii.
 */
std::string hex_to_char_filename( std::string& f );

/**
 * Given a file, return root filename in %d format, and frame passed if
 * sequence.  If not sequence fileroot, return file as is.
 *
 * @param fileroot    fileroot in %*d format or file
 * @param file        original file
 * @param change_view whether to replace views with %V or %v
 *
 * @return true if potential sequence, false if not
 */
bool fileroot( std::string& fileroot, const std::string& file,
               const bool change_view = true,
               const bool change_frame = true );

/**
 * Given a filename of a possible sequence, split it into
 * root name, frame string, and extension
 *
 * @param root    root name of file sequence (root.)
 * @param frame   frame part of file name    (%d)
 * @param view    left or right for stereo images.  Empty if not stereo.
 * @param ext     extension of file sequence (.ext)
 * @param file    original filename, potentially part of a sequence.
 * @param change_view whether to change view for %V or %v.
 *
 * @return true if a sequence, false if not.
 */
bool split_sequence(
    std::string& root, std::string& frame,
    std::string& view,
    std::string& ext, const std::string& file,
    const bool change_view = true,
    const bool change_frame = true
);

/**
 * Obtain the frame range of a sequence by scanning the directory where it
 * resides.
 *
 * @param firstFrame   first frame of sequence
 * @param lastFrame    last  frame of sequence
 * @param file         fileroot of sequence ( Example: mray.%04d.exr )
 * @param error        log errors on the log window
 *
 * @return true if sequence limits found, false if not.
 */
bool get_sequence_limits( int64_t& firstFrame,
                          int64_t& lastFrame,
                          std::string& file,
                          const bool error = true );

/**
 * Given a filename extension, return whether the extension is
 * from a movie format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible movie, false if not.
 */
bool is_valid_movie( const char* ext );

/**
 * Given a filename extension, return whether the extension is
 * from an audio format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible audio file, false if not.
 */
bool is_valid_audio( const char* ext );

/**
 * Given a filename extension, return whether the extension is
 * from a subtitle format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible subtitle file, false if not.
 */
bool is_valid_subtitle( const char* ext );

/**
 * Given a filename extension, return whether the extension is
 * from a picture format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible picture, false if not.
 */
bool is_valid_picture( const char* ext );

/**
 * Given a single image filename, return whether the image is
 * a sequence on disk (ie. there are several images named with a
 * similar convention)
 *
 * @param file Filename of image
 *
 * @return true if a possible sequence, false if not.
 */
bool is_valid_sequence( const char* file );

/**
 * Given a single filename, return whether the file is
 * a directory on disk
 *
 * @param file Filename or Directory
 *
 * @return true if a directory, false if not.
 */
bool is_directory( const char* file );


/**
 * Given a frame string like "0020", return the number of
 * padded digits
 *
 * @param frame a string like "0020" or "14".
 *
 * @return number of padded digits (4 for 0020, 1 for 14 ).
 */
int  padded_digits( const std::string& frame );


std::string get_short_view( bool left );
std::string get_long_view( bool left );

//! Parse a directory and return all movies, sequences and audios found there
void parse_directory( const std::string& directory,
                      stringArray& movies, stringArray& sequences, stringArray& audios );

// Parse a %v or %V fileroot and return the appropiate view name.
std::string parse_view( const std::string& fileroot, bool left = true );

std::string relative_path( const std::string& path, const std::string& parent,
                           const bool use_relative_paths = true );


}  // namespace mrv
