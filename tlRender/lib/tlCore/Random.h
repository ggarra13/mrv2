// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace math
    {
        //! Random numbers.
        class Random
        {
        public:
            Random();

            ~Random();

            //! Get a random number between zero and one.
            float get();

            //! Get a random number between zero and the given value.
            float get(float);

            //! Get a random number between zero and the given value.
            int get(int);

            //! Get a random number between two values.
            float get(float min, float max);

            //! Get a random number between two values.
            int get(int min, int max);

            //! Get a random item from a list.
            template <typename T> const T& get(const std::vector<T>&);

            //! Seed the random value generator.
            void setSeed(unsigned int);

            //! Seed the random value generator with the current time.
            void setSeed();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace math
} // namespace tl

#include <tlCore/RandomInline.h>
