// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    namespace time
    {
        //! Timer.
        class Timer : public std::enable_shared_from_this<Timer>
        {
            TLRENDER_NON_COPYABLE(Timer);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Timer();

        public:
            ~Timer();

            //! Create a new timer.
            static std::shared_ptr<Timer>
            create(const std::shared_ptr<system::Context>&);

            //! Does the timer repeat?
            bool isRepeating() const;

            //! Set whether the timer repeats.
            void setRepeating(bool);

            //! Start the timer.
            void start(
                const std::chrono::microseconds&,
                const std::function<void(void)>&);

            //! Start the timer.
            void start(
                const std::chrono::microseconds&,
                const std::function<void(
                    const std::chrono::steady_clock::time_point&,
                    const std::chrono::microseconds&)>&);

            //! Stop the timer.
            void stop();

            //! Is the timer active?
            bool isActive() const;

            //! Get the timeout.
            const std::chrono::microseconds& getTimeout() const;

            void tick();

        private:
            TLRENDER_PRIVATE();
        };

        //! Timer system.
        class TimerSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(TimerSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            TimerSystem();

        public:
            virtual ~TimerSystem();

            static std::shared_ptr<TimerSystem>
            create(const std::shared_ptr<system::Context>&);

            void addTimer(const std::shared_ptr<Timer>&);

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace time
} // namespace tl
