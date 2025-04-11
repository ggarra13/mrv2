// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/TimeTest.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>
#include <tlCore/Timer.h>

#include <sstream>

using namespace tl::time;

namespace tl
{
    namespace core_tests
    {
        TimeTest::TimeTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::TimeTest", context)
        {
        }

        std::shared_ptr<TimeTest>
        TimeTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TimeTest>(new TimeTest(context));
        }

        void TimeTest::run()
        {
            _otime();
            _sleep();
            _util();
            _keycode();
            _timecode();
            _timer();
            _serialize();
        }

        void TimeTest::_otime()
        {
            {
                std::stringstream ss;
                ss << "Invalid time: " << invalidTime;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Invalid time range: " << invalidTimeRange;
                _print(ss.str());
            }
            {
                TLRENDER_ASSERT(!isValid(invalidTime));
                TLRENDER_ASSERT(isValid(otime::RationalTime(24.0, 24.0)));
            }
            {
                TLRENDER_ASSERT(!isValid(invalidTimeRange));
                TLRENDER_ASSERT(isValid(otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0))));
            }
            {
                const otime::TimeRange a(
                    otime::RationalTime(24.0, 24.0),
                    otime::RationalTime(24.0, 24.0));
                TLRENDER_ASSERT(compareExact(a, a));
                const otime::TimeRange b(
                    otime::RationalTime(1.0, 1.0),
                    otime::RationalTime(1.0, 1.0));
                TLRENDER_ASSERT(a == b);
                TLRENDER_ASSERT(!compareExact(a, b));
            }
        }

        void TimeTest::_sleep()
        {
            sleep(std::chrono::microseconds(1000));
        }

        void TimeTest::_util()
        {
            {
                struct Data
                {
                    otime::TimeRange range;
                    std::vector<otime::RationalTime> frames;
                };
                const std::vector<Data> data = {
                    Data({time::invalidTimeRange, {}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 24.0),
                             otime::RationalTime(1.0, 24.0)),
                         {otime::RationalTime(0.0, 24.0)}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 24.0),
                             otime::RationalTime(3.0, 24.0)),
                         {otime::RationalTime(0.0, 24.0),
                          otime::RationalTime(1.0, 24.0),
                          otime::RationalTime(2.0, 24.0)}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 1.0),
                             otime::RationalTime(1.0, 1.0)),
                         {otime::RationalTime(0.0, 1.0)}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 1.0),
                             otime::RationalTime(3.0, 1.0)),
                         {otime::RationalTime(0.0, 1.0),
                          otime::RationalTime(1.0, 1.0),
                          otime::RationalTime(2.0, 1.0)}})};
                for (const auto& i : data)
                {
                    const auto frames = time::frames(i.range);
                    TLRENDER_ASSERT(frames == i.frames);
                }
            }
            {
                struct Data
                {
                    otime::TimeRange range;
                    std::vector<otime::TimeRange> seconds;
                };
                const std::vector<Data> data = {
                    Data({time::invalidTimeRange, {}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 24.0),
                             otime::RationalTime(24.0, 24.0)),
                         {otime::TimeRange(
                             otime::RationalTime(0.0, 24.0),
                             otime::RationalTime(24.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(0.0, 24.0),
                             otime::RationalTime(72.0, 24.0)),
                         {otime::TimeRange(
                              otime::RationalTime(0.0, 24.0),
                              otime::RationalTime(24.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(24.0, 24.0),
                              otime::RationalTime(24.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(48.0, 24.0),
                              otime::RationalTime(24.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(12.0, 24.0),
                             otime::RationalTime(12.0, 24.0)),
                         {otime::TimeRange(
                             otime::RationalTime(12.0, 24.0),
                             otime::RationalTime(12.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(12.0, 24.0),
                             otime::RationalTime(24.0, 24.0)),
                         {otime::TimeRange(
                              otime::RationalTime(12.0, 24.0),
                              otime::RationalTime(12.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(24.0, 24.0),
                              otime::RationalTime(12.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(23.0, 24.0),
                             otime::RationalTime(24.0, 24.0)),
                         {otime::TimeRange(
                              otime::RationalTime(23.0, 24.0),
                              otime::RationalTime(1.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(24.0, 24.0),
                              otime::RationalTime(23.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(-1.0, 24.0),
                             otime::RationalTime(24.0, 24.0)),
                         {otime::TimeRange(
                              otime::RationalTime(-1.0, 24.0),
                              otime::RationalTime(1.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(0.0, 24.0),
                              otime::RationalTime(23.0, 24.0))}}),
                    Data(
                        {otime::TimeRange(
                             otime::RationalTime(-1.0, 24.0),
                             otime::RationalTime(48.0, 24.0)),
                         {otime::TimeRange(
                              otime::RationalTime(-1.0, 24.0),
                              otime::RationalTime(1.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(0.0, 24.0),
                              otime::RationalTime(24.0, 24.0)),
                          otime::TimeRange(
                              otime::RationalTime(24.0, 24.0),
                              otime::RationalTime(23.0, 24.0))}})};
                for (const auto& i : data)
                {
                    const auto seconds = time::seconds(i.range);
                    TLRENDER_ASSERT(seconds == i.seconds);
                }
            }
            {
                struct Data
                {
                    double rate = 0.0;
                    std::pair<int, int> rational;
                };
                const std::vector<Data> data = {
                    Data({0.0, std::make_pair(0, 1)}),
                    Data({24.0, std::make_pair(24, 1)}),
                    Data({30.0, std::make_pair(30, 1)}),
                    Data({60.0, std::make_pair(60, 1)}),
                    Data({23.97602397602398, std::make_pair(24000, 1001)}),
                    Data({29.97002997002997, std::make_pair(30000, 1001)}),
                    Data({59.94005994005994, std::make_pair(60000, 1001)}),
                    Data({23.98, std::make_pair(24000, 1001)}),
                    Data({29.97, std::make_pair(30000, 1001)}),
                    Data({59.94, std::make_pair(60000, 1001)})};
                for (const auto& i : data)
                {
                    const auto rational = toRational(i.rate);
                    TLRENDER_ASSERT(
                        rational.first == i.rational.first &&
                        rational.second == i.rational.second);
                }
            }
        }

        void TimeTest::_keycode()
        {
            {
                const std::string s = keycodeToString(1, 2, 3, 4, 5);
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                stringToKeycode(s, id, type, prefix, count, offset);
                TLRENDER_ASSERT(1 == id);
                TLRENDER_ASSERT(2 == type);
                TLRENDER_ASSERT(3 == prefix);
                TLRENDER_ASSERT(4 == count);
                TLRENDER_ASSERT(5 == offset);
            }
            try
            {
                const std::string s = "...";
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                stringToKeycode(s, id, type, prefix, count, offset);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }

        void TimeTest::_timecode()
        {
            {
                const uint32_t t = timeToTimecode(1, 2, 3, 4);
                int hour = 0;
                int minute = 0;
                int second = 0;
                int frame = 0;
                timecodeToTime(t, hour, minute, second, frame);
                TLRENDER_ASSERT(1 == hour);
                TLRENDER_ASSERT(2 == minute);
                TLRENDER_ASSERT(3 == second);
                TLRENDER_ASSERT(4 == frame);
            }
            {
                const std::string s = "01:02:03:04";
                uint32_t t = 0;
                stringToTimecode(s, t);
                TLRENDER_ASSERT(s == timecodeToString(t));
            }
            try
            {
                const std::string s = "...";
                uint32_t t = 0;
                stringToTimecode(s, t);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }

        void TimeTest::_timer()
        {
            auto timer = Timer::create(_context);
            TLRENDER_ASSERT(!timer->isRepeating());
            timer->setRepeating(true);
            TLRENDER_ASSERT(timer->isRepeating());
            size_t count = 0;
            timer->start(
                std::chrono::milliseconds(100),
                [this, timer, &count]
                {
                    _print(string::Format("Timeout: {0}").arg(count));
                    ++count;
                    if (3 == count)
                    {
                        timer->stop();
                    }
                });
            TLRENDER_ASSERT(timer->isActive());
            TLRENDER_ASSERT(
                std::chrono::milliseconds(100) == timer->getTimeout());
            while (timer->isActive())
            {
                _context->tick();
            }
            timer->setRepeating(false);
            timer->start(
                std::chrono::milliseconds(100),
                [this](
                    const std::chrono::steady_clock::time_point&,
                    const std::chrono::microseconds& ms) {
                    _print(string::Format("Timeout: {0} microseconds")
                               .arg(ms.count()));
                });
            while (timer->isActive())
            {
                _context->tick();
            }
        }

        void TimeTest::_serialize()
        {
            {
                const otime::RationalTime t(1.0, 24.0);
                nlohmann::json json;
                to_json(json, t);
                otime::RationalTime t2 = invalidTime;
                from_json(json, t2);
                TLRENDER_ASSERT(t == t2);
            }
            {
                const auto t = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(1.0, 24.0));
                nlohmann::json json;
                to_json(json, t);
                otime::TimeRange t2 = invalidTimeRange;
                from_json(json, t2);
                TLRENDER_ASSERT(t == t2);
            }
            {
                const auto t = otime::RationalTime(1.0, 24.0);
                std::stringstream ss;
                ss << t;
                otime::RationalTime t2 = invalidTime;
                ss >> t2;
                TLRENDER_ASSERT(t == t2);
            }
            try
            {
                otime::RationalTime t = invalidTime;
                std::stringstream ss("...");
                ss >> t;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            {
                const auto t = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(1.0, 24.0));
                std::stringstream ss;
                ss << t;
                otime::TimeRange t2 = invalidTimeRange;
                ss >> t2;
                TLRENDER_ASSERT(t == t2);
            }
            try
            {
                otime::TimeRange t = invalidTimeRange;
                std::stringstream ss("...");
                ss >> t;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                otime::TimeRange t = invalidTimeRange;
                std::stringstream ss(".-.");
                ss >> t;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
