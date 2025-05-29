// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    //! Memory
    namespace memory
    {
        //! \name Sizes
        ///@{

        constexpr size_t kilobyte = 1024; //!< The number of bytes in a kilobyte
        constexpr size_t megabyte =
            kilobyte * 1024; //!< The number of bytes in a megabyte
        constexpr size_t gigabyte =
            megabyte * 1024; //!< The number of bytes in a gigabyte
        constexpr size_t terabyte =
            gigabyte * 1024; //!< The number of bytes in a terabyte

        ///@}

        //! \name Endian
        ///@{

        //! Endian type.
        enum class Endian {
            MSB, //!< Most significant byte first
            LSB, //!< Least significant byte first

            Count,
            First = MSB
        };
        TLRENDER_ENUM(Endian);
        TLRENDER_ENUM_SERIALIZE(Endian);

        //! Get the current endian.
        Endian getEndian();

        //! Get the opposite of the given endian.
        constexpr Endian opposite(Endian);

        //! Convert the endianness of a block of memory in place.
        void endian(void* in, size_t size, size_t wordSize);

        //! Convert the endianness of a block of memory.
        void endian(const void* in, void* out, size_t size, size_t wordSize);

        ///@}

        //! \name Bits
        ///@{

        //! Get the given bit.
        bool getBit(unsigned int, int bit);

        //! Set the given bit.
        unsigned int setBit(unsigned int, int bit);

        //! Clear the given bit.
        unsigned int clearBit(unsigned int, int bit);

        //! Toggle the given bit.
        unsigned int toggleBit(unsigned int, int bit);

        //! Convert bits to a string for debugging.
        std::string getBitString(uint8_t);

        //! Convert bits to a string for debugging.
        std::string getBitString(uint16_t);

        ///@}
    } // namespace memory
} // namespace tl

#include <tlCore/MemoryInline.h>
