// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <cstdint>

namespace tl
{
    namespace memory
    {
        inline Endian getEndian()
        {
            const uint32_t tmp = 1;
            return *(reinterpret_cast<const uint8_t*>(&tmp)) ? Endian::LSB
                                                             : Endian::MSB;
        }

        constexpr Endian opposite(Endian in)
        {
            return Endian::MSB == in ? Endian::LSB : Endian::MSB;
        }

        inline bool getBit(unsigned int value, int bit)
        {
            return (value >> bit) & 0x01;
        }

        inline unsigned int setBit(unsigned int value, int bit)
        {
            return value | (1 << bit);
        }

        inline unsigned int clearBit(unsigned int value, int bit)
        {
            return value & ~(1 << bit);
        }

        inline unsigned int toggleBit(unsigned int value, int bit)
        {
            return value ^ (1 << bit);
        }
    } // namespace memory
} // namespace tl
