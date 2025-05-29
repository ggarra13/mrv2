// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Random.h>

#include <ctime>
#include <random>

namespace tl
{
    namespace math
    {
        struct Random::Private
        {
            std::random_device rd;
            std::unique_ptr<std::mt19937> rng;
        };

        Random::Random() :
            _p(new Private)
        {
            TLRENDER_P();
            p.rng = std::make_unique<std::mt19937>(p.rd());
        }

        Random::~Random() {}

        float Random::get()
        {
            TLRENDER_P();
            std::uniform_int_distribution<uint32_t> uint_dist;
            return uint_dist(*(p.rng)) / static_cast<float>(uint_dist.max());
        }

        float Random::get(float value)
        {
            return value * get();
        }

        int Random::get(int value)
        {
            TLRENDER_P();
            std::uniform_int_distribution<uint32_t> uint_dist;
            const float r = static_cast<float>(uint_dist(*(p.rng))) /
                            static_cast<float>(uint_dist.max());
            return static_cast<int>(static_cast<float>(value + 1) * r);
        }

        float Random::get(float min, float max)
        {
            return min + (max - min) * get();
        }

        int Random::get(int min, int max)
        {
            TLRENDER_P();
            std::uniform_int_distribution<uint32_t> uint_dist;
            const float r = static_cast<float>(uint_dist(*(p.rng))) /
                            static_cast<float>(uint_dist.max());
            return min +
                   static_cast<int>(static_cast<float>(max - min + 1) * r);
        }

        void Random::setSeed(unsigned int value)
        {
            _p->rng->seed(value);
        }

        void Random::setSeed()
        {
            TLRENDER_P();
            const std::time_t t = std::time(nullptr);
            std::tm tm;
#if defined(_WINDOWS)
            localtime_s(&tm, &t);
#else  // _WINDOWS
            localtime_r(&t, &tm);
#endif // _WINDOWS
            setSeed(tm.tm_sec);
        }
    } // namespace math
} // namespace tl