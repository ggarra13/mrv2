// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MemoryTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Memory.h>

#include <sstream>

using namespace tl::memory;

namespace tl
{
    namespace core_tests
    {
        MemoryTest::MemoryTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::MemoryTest", context)
        {
        }

        std::shared_ptr<MemoryTest>
        MemoryTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MemoryTest>(new MemoryTest(context));
        }

        void MemoryTest::run()
        {
            _enums();
            _endian();
            _bits();
        }

        void MemoryTest::_enums()
        {
            _enum<Endian>("Endian", getEndianEnums);
        }

        void MemoryTest::_endian()
        {
            {
                std::stringstream ss;
                ss << "Current endian: " << getEndian();
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Opposite endian: " << opposite(getEndian());
                _print(ss.str());
            }
            {
                uint8_t u8 = 1;
                endian(&u8, 1, 1);
                TLRENDER_ASSERT(1 == u8);
            }
            {
                uint16_t u16 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u16);
                p[0] = 0;
                p[1] = 1;
                endian(&u16, 1, 2);
                TLRENDER_ASSERT(1 == p[0]);
                TLRENDER_ASSERT(0 == p[1]);
            }
            {
                uint32_t u32 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u32);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[3] = 3;
                endian(&u32, 1, 4);
                TLRENDER_ASSERT(3 == p[0]);
                TLRENDER_ASSERT(2 == p[1]);
                TLRENDER_ASSERT(1 == p[2]);
                TLRENDER_ASSERT(0 == p[3]);
            }
            {
                uint64_t u64 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u64);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[3] = 3;
                p[4] = 4;
                p[5] = 5;
                p[6] = 6;
                p[7] = 7;
                endian(&u64, 1, 8);
                TLRENDER_ASSERT(7 == p[0]);
                TLRENDER_ASSERT(6 == p[1]);
                TLRENDER_ASSERT(5 == p[2]);
                TLRENDER_ASSERT(4 == p[3]);
                TLRENDER_ASSERT(3 == p[4]);
                TLRENDER_ASSERT(2 == p[5]);
                TLRENDER_ASSERT(1 == p[6]);
                TLRENDER_ASSERT(0 == p[7]);
            }
            {
                uint8_t u8 = 0;
                uint8_t _u8 = 0;
                endian(&u8, &_u8, 1, 1);
                TLRENDER_ASSERT(u8 == _u8);
            }
            {
                uint16_t u16 = 0;
                uint16_t _u16 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u16);
                uint8_t* p2 = reinterpret_cast<uint8_t*>(&_u16);
                p[0] = 0;
                p[1] = 1;
                endian(&u16, &_u16, 1, 2);
                TLRENDER_ASSERT(p[0] == p2[1]);
                TLRENDER_ASSERT(p[1] == p2[0]);
            }
            {
                uint32_t u32 = 0;
                uint32_t _u32 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u32);
                uint8_t* p2 = reinterpret_cast<uint8_t*>(&_u32);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[3] = 3;
                endian(&u32, &_u32, 1, 4);
                TLRENDER_ASSERT(p[0] == p2[3]);
                TLRENDER_ASSERT(p[1] == p2[2]);
                TLRENDER_ASSERT(p[2] == p2[1]);
                TLRENDER_ASSERT(p[3] == p2[0]);
            }
            {
                uint64_t u64 = 0;
                uint64_t _u64 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u64);
                uint8_t* p2 = reinterpret_cast<uint8_t*>(&_u64);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[3] = 3;
                p[4] = 4;
                p[5] = 5;
                p[6] = 6;
                p[7] = 7;
                endian(&u64, &_u64, 1, 8);
                TLRENDER_ASSERT(p[0] == p2[7]);
                TLRENDER_ASSERT(p[1] == p2[6]);
                TLRENDER_ASSERT(p[2] == p2[5]);
                TLRENDER_ASSERT(p[3] == p2[4]);
                TLRENDER_ASSERT(p[4] == p2[3]);
                TLRENDER_ASSERT(p[5] == p2[2]);
                TLRENDER_ASSERT(p[6] == p2[1]);
                TLRENDER_ASSERT(p[7] == p2[0]);
            }
        }

        void MemoryTest::_bits()
        {
            {
                uint8_t a = 0;
                _print(getBitString(a));
                TLRENDER_ASSERT(!getBit(a, 0));
                a = setBit(a, 0);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 0));
                a = setBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 7));
                a = clearBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(!getBit(a, 7));
                a = toggleBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 7));
            }
            {
                uint16_t a = 0;
                _print(getBitString(a));
                TLRENDER_ASSERT(!getBit(a, 0));
                a = setBit(a, 0);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 0));
                a = setBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 7));
                a = clearBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(!getBit(a, 7));
                a = toggleBit(a, 7);
                _print(getBitString(a));
                TLRENDER_ASSERT(getBit(a, 7));
            }
        }
    } // namespace core_tests
} // namespace tl
