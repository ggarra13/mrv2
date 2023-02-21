// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/fl_utf8.h>

#include <tlCore/StringFormat.h>

#include <cinttypes>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvUtil.h"

extern "C"
{
#include <libavutil/avutil.h>
}

#include "mrvCore/mrvOS.h"
#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "seq";
}

namespace mrv
{

    bool is_valid_frame(const std::string& framespec)
    {
        if (framespec.size() > 17)
            return false;

        const char* c = framespec.c_str();
        if (*c == '.')
            ++c;

        for (; *c != 0; ++c)
        {
            if (*c == '+' || *c == '-' || (*c >= '0' && *c <= '9'))
                continue;

            return false;
        }

        return true;
    }

    bool is_valid_frame_spec(std::string& framespec)
    {
        const char* c;
        if (framespec.substr(0, 1) == ".")
            c = framespec.c_str() + 1;
        else
            c = framespec.c_str();

        if (framespec.size() > 17)
            return false;

        if (*c == '%')
        {
            bool d = false;
            bool l = false;
            for (++c; *c != 0; ++c)
            {
                if ((*c >= '0' && *c <= '9'))
                    continue;
                if (!d && *c == 'd')
                {
                    d = true;
                    continue;
                }
                if (!l && *c == 'l')
                {
                    l = true;
                    continue;
                }
                return false;
            }
            return true;
        }
        else if (*c == '#' || *c == '@')
        {
            char t = *c;
            for (++c; *c != 0; ++c)
            {
                if (*c != t)
                    return false;
            }
            return true;
        }

        int idx = 0;
        bool range_found = false;
        for (++c; *c != 0; ++c)
        {
            if (!range_found)
            {
                ++idx;
                if (*c == '-')
                {
                    range_found = true;
                    continue;
                }
            }
            if (*c != '+' && *c >= '0' && *c <= '9')
                continue;

            return false;
        }

        framespec = framespec.substr(0, idx);

        return range_found;
    }

    std::string get_short_view(bool left)
    {
        const char* pairs = fl_getenv("MRV_STEREO_CHAR_PAIRS");
        if (!pairs)
            pairs = "L:R";

        std::string view = pairs;
        size_t idx = view.find(':');
        if (idx == std::string::npos)
        {
            // LOG_ERROR( "MRV_STEREO_CHAR_PAIRS does not have two letters
            // separated by colon" );
            if (left)
                return "L";
            else
                return "R";
        }

        if (left)
            return view.substr(0, idx);
        else
            return view.substr(idx + 1, view.size());
    }

    std::string get_long_view(bool left)
    {
        const char* pairs = fl_getenv("MRV_STEREO_NAME_PAIRS");
        if (!pairs)
            pairs = "left:right";

        std::string view = pairs;
        size_t idx = view.find(':');
        if (idx == std::string::npos)
        {
            LOG_ERROR("MRV_STEREO_NAME_PAIRS does not have two names separated "
                      "by colon");
            if (left)
                return "left";
            else
                return "right";
        }

        if (left)
            return view.substr(0, idx);
        else
            return view.substr(idx + 1, view.size());
    }

    bool replace_view(std::string& view)
    {
        if (view.empty())
            return false;

        if (view.substr(view.size() - 1, view.size()) == ".")
            view = view.substr(0, view.size() - 1);

        if (view == "%v" || view == get_short_view(true) ||
            view == get_short_view(false))
        {
            view = "%v";
            return true;
        }

        if (view == "%V" || view == get_long_view(true) ||
            view == get_long_view(false))
        {
            view = "%V";
            return true;
        }
        return false;
    }

    bool is_valid_view(std::string view)
    {
        return replace_view(view);
    }

    /**
     * Given a filename of a possible sequence, split it into
     * root name, frame string, view, and extension
     *
     * @param root        root name of file sequence
     * @param frame       frame part of file (must be @ or # or %d or a number)
     * @param view        view of image (%v or %V from left or right or L and R
     * )
     * @param ext         extension of file sequence
     * @param file        original filename, potentially part of a sequence.
     * @param change_view change view to %v or %V if left/right or L/R is found.
     * @param change_frame change frame into frame spec
     *
     * @return true if a sequence, false if not.
     */
    bool split_sequence(
        std::string& root, std::string& frame, std::string& view,
        std::string& ext, const std::string& file, const bool change_view,
        const bool change_frame)
    {
        std::string f = file;

        root = frame = view = ext = "";

        const char* e = f.c_str();
        const char* i = e + f.size() - 1;
        for (; i >= e; --i)
        {
            if (*i == '/' || *i == '\\')
                break;
        }

        size_t len = i - e + 1;
        f = f.substr(len, f.size());

        stringArray periods;
        split(periods, f, '.');

        if (periods.size() == 4)
        {
            root = file.substr(0, len) + periods[0] + ".";
            if (root.find("file://") == 0)
                root = root.substr(7, root.size());
            view = periods[1];
            frame = periods[2];
            ext = '.' + periods[3];

            if (change_view)
            {
                bool ok = replace_view(view);
            }

            if (!view.empty())
                view += ".";
            if (mrv::is_valid_movie(ext.c_str()) ||
                mrv::is_valid_audio(ext.c_str()))
            {
                if (frame != "" && (ext == ".gif" || ext == ".GIF"))
                    return true;
                root += view + frame + ext;
                view = "";
                frame = "";
                ext = "";
                return false;
            }
            else
            {
                if (is_valid_frame(frame) || is_valid_frame_spec(frame))
                    return true;
                return false;
            }
        }
        else if (periods.size() == 3)
        {
            root = file.substr(0, len) + periods[0] + ".";
            if (root.find("file://") == 0)
                root = root.substr(7, root.size());
            frame = periods[1];
            ext = '.' + periods[2];
            if (mrv::is_valid_movie(ext.c_str()) ||
                mrv::is_valid_audio(ext.c_str()))
            {
                if (frame != "" && (ext == ".gif" || ext == ".GIF"))
                    return true;

                if (!mrv::is_valid_frame(frame) &&
                    !mrv::is_valid_frame_spec(frame) &&
                    mrv::is_valid_view(frame))
                {
                    view = periods[1];
                    if (change_view)
                        replace_view(view);
                    if (change_frame)
                        frame = "";
                }
                root += view;
                root += frame;
                root += ext;
                frame = ext = view = "";
                return false;
            }
            if (!change_frame)
                return true;
        }

        f = file;

        int idx[2];
        int count = 0; // number of periods found (from end)

        int minus_idx = -1; // index where last - sign was found.
        int minus = 0;      // number of minus signs found

        e = f.c_str();
        i = e + f.size() - 1;
        for (; i >= e; --i)
        {
            if (*i == '/' || *i == '\\')
                break;
            if (*i == '.' || (count > 0 && (*i == '_')))
            {
                idx[count] = (int)(i - e);
                ++count;
                if (count == 2)
                    break;
                continue;
            }

            if (count == 1 &&
                (*i != '@' && *i != '#' && *i != 'd' && *i != 'l' &&
                 *i != '%' && *i != '-' && *i != 'I' && (*i < '0' || *i > '9')))
                break;
            if (count == 1 && *i == '-')
            {
                minus_idx = (int)(i - e);
                minus++;
            }
        }

        if (count == 1 && minus == 1)
        {
            idx[count] = minus_idx;
            ++count;
        }

        if (count == 0)
            return false;

        if (count == 2 && minus < 2)
        {
            root = f.substr(0, idx[1] + 1);
            if (root.find("file://") == 0)
                root = root.substr(7, root.size());
            frame = f.substr(idx[1] + 1, idx[0] - idx[1] - 1);
            ext = f.substr(idx[0], file.size() - idx[0]);

            bool ok = is_valid_frame(frame);
            if (ok && (!is_valid_movie(ext.c_str()) ||
                       mrv::is_valid_audio(ext.c_str())))
            {
                return true;
            }

            ok = is_valid_frame(ext);
            if (ok)
            {
                frame = ext;
                ext.clear();
            }

            if (is_valid_movie(ext.c_str()) || mrv::is_valid_audio(ext.c_str()))
            {
                if (frame != "" && (ext == ".gif" || ext == ".GIF"))
                    return true;

                root = "";
                return false;
            }

            ok = is_valid_frame_spec(frame);
            return ok;
        }
        else
        {
            root = f.substr(0, idx[0] + 1);
            if (root.find("file://") == 0)
                root = root.substr(7, root.size());
            ext = f.substr(idx[0] + 1, file.size());

            if (is_valid_movie(ext.c_str()) || is_valid_audio(ext.c_str()))
            {
                frame = "";
                return false;
            }

            bool ok = is_valid_frame_spec(ext);
            if (ok)
            {
                frame = ext;
                ext.clear();
                return true;
            }

            ok = is_valid_frame(ext);
            if (ok)
            {
                frame = ext;
                ext.clear();
                return false;
            }

            //
            // Handle image0001.exr
            //
            std::string tmp = '.' + ext;
            bool valid =
                is_valid_movie(tmp.c_str()) || mrv::is_valid_audio(ext.c_str());
            size_t len = root.size();
            if (len >= 2 && !valid)
            {
                if (root.find("file://") == 0)
                    root = root.substr(7, root.size());
                size_t pos;
                std::string fspec;
                if ((pos = root.rfind('%')) != std::string::npos ||
                    (pos = root.find('@')) != std::string::npos ||
                    (pos = root.rfind('#')) != std::string::npos)
                {
                    fspec = root.substr(pos, root.size() - pos - 1);
                    if (is_valid_frame_spec(fspec))
                    {
                        root = root.substr(0, pos);
                        frame = fspec;
                        ext = tmp;
                        return true;
                    }
                }

                int count = 0;
                int i = (int)len - 2; // account for period
                const char* c = root.c_str();
                while (c[i] >= '0' && c[i] <= '9')
                {
                    --i;
                    ++count;
                    if (i == -1)
                        break;
                }

                if (count > 0 && i < int(root.size() - 2))
                {
                    ++i;
                    frame = root.substr(i, count);
                    root = root.substr(0, i);

                    size_t pos = root.rfind('/');
                    size_t pos2 = root.rfind('\\');
                    size_t pos3 = root.find(':');
                    if (pos == std::string::npos ||
                        (pos2 != std::string::npos && pos2 > pos))
                        pos = pos2;
                    if (pos == std::string::npos ||
                        (pos3 != std::string::npos && pos3 > pos))
                        pos = pos3;

                    if (root.empty() || pos != std::string::npos)
                    {
                        // Check if filename is empty, like image: 324.exr
                        std::string file;
                        if (!root.empty())
                            file = root.substr(pos + 1, root.size());
                        if (file.empty())
                        {
                            if (is_valid_frame(frame) ||
                                is_valid_frame_spec(frame))
                            {
                                ext = tmp;
                                return true;
                            }
                            else
                            {
                                root = frame = ext = "";
                            }
                            return false;
                        }
                    }
                    ext = tmp;
                    return true;
                }
            }

            frame = "";
            return false;
        }
    }

    bool get_sequence_limits(
        int64_t& frameStart, int64_t& frameEnd, std::string& fileroot,
        const bool error)
    {
        if ((frameStart != AV_NOPTS_VALUE && frameEnd != AV_NOPTS_VALUE))
        {
            return true;
        }

        fs::path file = fs::path(fileroot.c_str());
        fs::path dir = file.parent_path();

        char buf[1024];
        if (dir.string() == "")
        {
            dir = fs::current_path();
        }

        if ((!fs::exists(dir)) || (!fs::is_directory(dir)))
        {
            if (error)
            {
                std::string err = tl::string::Format(_("Directory {0} does not "
                                                       "exist or no directory"))
                                      .arg(dir);
                LOG_ERROR(err);
            }
            return false;
        }

        // Check if sequence is in ILM format first  ( image.1-30.exr )
        stringArray tokens;
        split(tokens, fileroot, '.');
        if (tokens.size() > 2)
        {
            int idx = 2;

            const std::string& range = tokens[tokens.size() - idx];

            if (mrv::matches_chars(range.c_str(), "0123456789-"))
            {
                stringArray frames;
                split(frames, range, '-');
                if (frames.size() > 1 && !frames[0].empty())
                {
                    unsigned digits = (unsigned)frames[0].size();

                    frameStart = atoi(frames[0].c_str());
                    frameEnd = atoi(frames[1].c_str());

                    if (frameEnd < frameStart)
                    {
                        frameEnd = frameStart = AV_NOPTS_VALUE;
                        return false;
                    }

                    stringArray::iterator i = tokens.begin();
                    stringArray::iterator e = tokens.end();
                    fileroot = tokens[0];
                    ++i;
                    for (; i != e; ++i)
                    {
                        fileroot += ".";
                        if (*i == range)
                        {
                            char buf[64];
                            const char* pr = PRId64;
                            if (digits < 10)
                                pr = "d";
                            snprintf(buf, 64, "%%0%d%s", digits, pr);
                            fileroot += buf;
                        }
                        else
                        {
                            fileroot += *i;
                        }
                    }

                    return true;
                }
            }
        }

        std::string root, frame, view, ext;
        if (!split_sequence(root, frame, view, ext, file.filename().string()))
        {
            return false; // NOT a recognized sequence
        }

        std::string croot, cview, cframe, cext;
        unsigned pad = 0;
        if (is_valid_frame_spec(frame))
        {
            pad = padded_digits(frame);
        }

        fs::directory_iterator e; // default constructor yields path iter. end
        for (fs::directory_iterator i(dir); i != e; ++i)
        {
            if (!fs::exists(*i) || fs::is_directory(*i))
                continue;

            std::string tmp = (*i).path().filename().generic_string();

            // Do not continue on false return of split_sequence
            if (!split_sequence(croot, cframe, cview, cext, tmp))
            {
                continue;
            }

            if (cext != ext || croot != root || cview != view)
            {
                continue; // not this sequence
            }

            if (cframe[0] == '0' && cframe.size() > 1 && pad == 0)
                pad = (unsigned)cframe.size();

            int64_t f = atoi(cframe.c_str());

            if (f < frameStart || frameStart == AV_NOPTS_VALUE)
                frameStart = f;
            if (f > frameEnd || frameEnd == AV_NOPTS_VALUE)
                frameEnd = f;
        }

        const char* prdigits = PRId64;
        if (pad < 10)
            prdigits = "d";

        snprintf(buf, 1024, "%%0%d%s", pad, prdigits);

        if (!split_sequence(root, frame, view, ext, fileroot))
        {
            return false;
        }

        fileroot = root;
        fileroot += view;
        fileroot += buf;
        fileroot += ext;

        return true;
    }

    bool fileroot(
        std::string& fileroot, const std::string& file, const bool change_view,
        const bool change_frame)
    {
        std::string root, frame, view, ext;

        bool ok = split_sequence(
            root, frame, view, ext, file, change_view, change_frame);
        if (!ok || frame == "" || is_valid_movie(ext.c_str()) ||
            mrv::is_valid_audio(ext.c_str()))
        {
            fileroot = file;
            return false;
        }

        if (!change_frame)
        {
            root += view;
            root += frame;
            root += ext;
            fileroot = root;
            return true;
        }

        const char* digits = PRId64;
        int pad = padded_digits(frame);
        if (pad < 10)
        {
            digits = "d";
        }

        char full[1024];
        if (pad == 0)
        {
            snprintf(
                full, 1024, "%s%s%%%s%s", root.c_str(), view.c_str(), digits,
                ext.c_str());
        }
        else
        {
            snprintf(
                full, 1024, "%s%s%%0%d%s%s", root.c_str(), view.c_str(), pad,
                digits, ext.c_str());
        }

        fileroot = full;
        return true;
    }

    void parse_directory(
        const std::string& dir, stringArray& movies, stringArray& sequences,
        stringArray& audios)
    {

        // default constructor yields path iter. end
        stringArray files;
        fs::directory_iterator e;
        for (fs::directory_iterator i(dir); i != e; ++i)
        {
            if (!fs::exists(*i) || fs::is_directory(*i))
                continue;

            std::string file = (*i).path().string();
            files.push_back(file);
        }
        std::sort(files.begin(), files.end());

        std::string root, frame, view, ext;
        SequenceList tmpseqs;

        for (const auto& file : files)
        {
            bool ok = mrv::split_sequence(root, frame, view, ext, file);
            if (mrv::is_valid_movie(ext.c_str()))
                movies.push_back(file);
            else if (mrv::is_valid_audio(ext.c_str()))
                audios.push_back(file);
            else if (ok)
            {
                Sequence s;
                s.root = root;
                s.view = view;
                s.number = frame;
                s.ext = ext;

                tmpseqs.push_back(s);
            }
        }

        //
        // Then, sort sequences and collapse them into a single file entry
        //
        std::sort(tmpseqs.begin(), tmpseqs.end(), SequenceSort());

        std::string first;
        std::string number;
        int padding = -1;

        for (const auto& i : tmpseqs)
        {

            const char* s = i.number.c_str();
            int z = 0;
            for (; *s == '0'; ++s)
                ++z;

            if (i.root != root || i.view != view || i.ext != ext ||
                (padding != z && z != padding - 1))
            {
                // New sequence
                root = i.root;
                padding = z;
                number = first = i.number;
                view = i.view;
                ext = i.ext;

                std::string file = root;
                file += first;
                file += view;
                file += ext;
                sequences.push_back(file);
            }
            else
            {
                padding = z;
                number = i.number;
            }
        }
    }

    // Preferences::uiMain->uiPrefs->uiPrefsRelativePaths->value()

    std::string relative_path(
        const std::string& root, const std::string& parent,
        const bool use_relative_paths)
    {

        std::string path = root;

        if (use_relative_paths)
        {
            fs::path parentPath = parent;
            parentPath = parentPath.parent_path();
            fs::path childPath = root;
            fs::path relativePath = fs::relative(childPath, parentPath);
            path = relativePath.string();
            if (path.empty())
                path = root;
        }

        std::replace(path.begin(), path.end(), '\\', '/');

        return path;
    }

    std::string parse_view(const std::string& root, bool left)
    {
        size_t idx = root.find("%V");
        std::string tmp = root;
        if (idx != std::string::npos)
        {
            tmp = root.substr(0, idx);
            tmp += get_long_view(left);
            tmp += root.substr(idx + 2, root.size());
        }
        else
        {
            idx = root.find("%v");
            if (idx != std::string::npos)
            {
                tmp = root.substr(0, idx);
                tmp += get_short_view(left);
                tmp += root.substr(idx + 2, root.size());
            }
        }
        return tmp;
    }

} // namespace mrv
