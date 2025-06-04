// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline MemoryRead::MemoryRead() {}

        inline MemoryRead::MemoryRead(const uint8_t* p, size_t size) :
            p(p),
            size(size)
        {
        }

        inline bool MemoryRead::operator==(const MemoryRead& other) const
        {
            return p == other.p && size == other.size;
        }

        inline bool MemoryRead::operator!=(const MemoryRead& other) const
        {
            return !(*this == other);
        }
    } // namespace file
} // namespace tl
