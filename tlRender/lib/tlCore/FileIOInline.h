// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline MemoryRead::MemoryRead(const std::shared_ptr<void>& f,
                                      const uint8_t* p, size_t size) :
            f(f),
            p(p),
            size(size)
        {}

        inline bool MemoryRead::operator==(const MemoryRead& other) const
        {
            return
                f == other.f &&
                p == other.p &&
                size == other.size;
        }

        inline bool MemoryRead::operator!=(const MemoryRead& other) const
        {
            return !(*this == other);
        }
    } // namespace file
} // namespace tl
