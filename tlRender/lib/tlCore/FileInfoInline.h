// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline const Path& FileInfo::getPath() const
        {
            return _path;
        }

        inline Type FileInfo::getType() const
        {
            return _type;
        }

        inline uint64_t FileInfo::getSize() const
        {
            return _size;
        }

        inline int FileInfo::getPermissions() const
        {
            return _permissions;
        }

        inline time_t FileInfo::getTime() const
        {
            return _time;
        }
    } // namespace file
} // namespace tl
