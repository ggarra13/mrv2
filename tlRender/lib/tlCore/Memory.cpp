// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Memory.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <cstring>

namespace tl
{
    namespace memory
    {
        TLRENDER_ENUM_IMPL(Endian, "MSB", "LSB");
        TLRENDER_ENUM_SERIALIZE_IMPL(Endian);

        void endian(void* in, size_t size, size_t wordSize)
        {
            uint8_t* p = reinterpret_cast<uint8_t*>(in);
            uint8_t tmp;
            switch (wordSize)
            {
            case 2:
                while (size--)
                {
                    tmp = p[0];
                    p[0] = p[1];
                    p[1] = tmp;
                    p += 2;
                }
                break;
            case 4:
                while (size--)
                {
                    tmp = p[0];
                    p[0] = p[3];
                    p[3] = tmp;
                    tmp = p[1];
                    p[1] = p[2];
                    p[2] = tmp;
                    p += 4;
                }
                break;
            case 8:
                while (size--)
                {
                    tmp = p[0];
                    p[0] = p[7];
                    p[7] = tmp;
                    tmp = p[1];
                    p[1] = p[6];
                    p[6] = tmp;
                    tmp = p[2];
                    p[2] = p[5];
                    p[5] = tmp;
                    tmp = p[3];
                    p[3] = p[4];
                    p[4] = tmp;
                    p += 8;
                }
                break;
            default:
                break;
            }
        }

        void endian(const void* in, void* out, size_t size, size_t wordSize)
        {
            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);
            uint8_t* outP = reinterpret_cast<uint8_t*>(out);
            switch (wordSize)
            {
            case 2:
                while (size--)
                {
                    outP[0] = inP[1];
                    outP[1] = inP[0];
                    inP += 2;
                    outP += 2;
                }
                break;
            case 4:
                while (size--)
                {
                    outP[0] = inP[3];
                    outP[1] = inP[2];
                    outP[2] = inP[1];
                    outP[3] = inP[0];
                    inP += 4;
                    outP += 4;
                }
                break;
            case 8:
                while (size--)
                {
                    outP[0] = inP[7];
                    outP[1] = inP[6];
                    outP[2] = inP[5];
                    outP[3] = inP[4];
                    outP[4] = inP[3];
                    outP[5] = inP[2];
                    outP[6] = inP[1];
                    outP[7] = inP[0];
                    inP += 8;
                    outP += 8;
                }
                break;
            default:
                std::memcpy(out, in, size * wordSize);
                break;
            }
        }

        std::string getBitString(uint8_t value)
        {
            std::string out;
            for (int i = 0; i < 8; ++i)
            {
                out.push_back((value & 0x01) ? '1' : '0');
                value = value >> 1;
            }
            return out;
        }

        std::string getBitString(uint16_t value)
        {
            std::string out;
            for (int i = 0; i < 16; ++i)
            {
                out.push_back((value & 0x01) ? '1' : '0');
                value = value >> 1;
            }
            return out;
        }
    } // namespace memory
} // namespace tl
