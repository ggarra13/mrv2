// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Time.h>

#include <tlCore/Error.h>
#include <tlCore/Locale.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace time
    {
        std::vector<otime::RationalTime> frames(const otime::TimeRange& value)
        {
            std::vector<otime::RationalTime> out;
            if (isValid(value))
            {
                const auto start = value.start_time();
                const auto end = value.end_time_exclusive();
                const auto inc =
                    otime::RationalTime(1.0, value.duration().rate());
                for (auto time = start; time < end; time += inc)
                {
                    out.push_back(time);
                }
            }
            return out;
        }

        std::vector<otime::TimeRange> seconds(const otime::TimeRange& value)
        {
            std::vector<otime::TimeRange> out;
            if (value != invalidTimeRange)
            {
                const otime::TimeRange seconds(
                    value.start_time().rescaled_to(1.0),
                    value.duration().rescaled_to(1.0));
                for (double t = std::floor(seconds.start_time().value());
                     t < std::ceil(seconds.end_time_exclusive().value());
                     t += 1.0)
                {
                    const double start =
                        std::max(t, seconds.start_time().value());
                    const double end =
                        std::min(t + 1.0, seconds.end_time_exclusive().value());
                    const otime::TimeRange second(
                        otime::RationalTime(start, 1.0),
                        otime::RationalTime(end - start, 1.0));
                    const otime::TimeRange scaled(
                        second.start_time().rescaled_to(
                            value.duration().rate()),
                        second.duration().rescaled_to(value.duration().rate()));
                    out.push_back(scaled);
                }
            }
            return out;
        }

        void sleep(
            const std::chrono::microseconds& value,
            const std::chrono::steady_clock::time_point& t0,
            const std::chrono::steady_clock::time_point& t1)
        {
            const std::chrono::duration<double> diff = t1 - t0;
            const long long diffClamped = math::clamp(
                static_cast<int64_t>(diff.count() * 1000000),
                static_cast<int64_t>(0), static_cast<int64_t>(value.count()));
            time::sleep(std::chrono::microseconds(value.count() - diffClamped));
        }

        std::pair<int, int> toRational(double value)
        {
            const std::array<std::pair<int, int>, 6> common = {
                std::make_pair(24, 1),       std::make_pair(30, 1),
                std::make_pair(60, 1),       std::make_pair(24000, 1001),
                std::make_pair(30000, 1001), std::make_pair(60000, 1001)};
            const double tolerance = 0.01;
            for (const auto& i : common)
            {
                const double diff =
                    fabs(value - i.first / static_cast<double>(i.second));
                if (diff < tolerance)
                {
                    return i;
                }
            }
            return std::make_pair(static_cast<int>(value), 1);
        }

        std::string
        keycodeToString(int id, int type, int prefix, int count, int offset)
        {
            std::vector<std::string> list;
            list.push_back(std::to_string(id));
            list.push_back(std::to_string(type));
            list.push_back(std::to_string(prefix));
            list.push_back(std::to_string(count));
            list.push_back(std::to_string(offset));
            return string::join(list, ":");
        }

        void stringToKeycode(
            const std::string& string, int& id, int& type, int& prefix,
            int& count, int& offset)
        {
            const auto pieces = string::split(string, ':');
            if (pieces.size() != 5)
            {
                throw error::ParseError();
            }
            id = std::stoi(pieces[0]);
            type = std::stoi(pieces[1]);
            prefix = std::stoi(pieces[2]);
            count = std::stoi(pieces[3]);
            offset = std::stoi(pieces[4]);
        }

        void timecodeToTime(
            uint32_t in, int& hour, int& minute, int& seconds, int& frame)
        {
            hour = (in >> 28 & 0x0f) * 10 + (in >> 24 & 0x0f);
            minute = (in >> 20 & 0x0f) * 10 + (in >> 16 & 0x0f);
            seconds = (in >> 12 & 0x0f) * 10 + (in >> 8 & 0x0f);
            frame = (in >> 4 & 0x0f) * 10 + (in >> 0 & 0x0f);
        }

        uint32_t timeToTimecode(int hour, int minute, int seconds, int frame)
        {
            return (hour / 10 & 0x0f) << 28 | (hour % 10 & 0x0f) << 24 |
                   (minute / 10 & 0x0f) << 20 | (minute % 10 & 0x0f) << 16 |
                   (seconds / 10 & 0x0f) << 12 | (seconds % 10 & 0x0f) << 8 |
                   (frame / 10 & 0x0f) << 4 | (frame % 10 & 0x0f) << 0;
        }

        std::string timecodeToString(uint32_t in)
        {
            int hour = 0;
            int minute = 0;
            int second = 0;
            int frame = 0;
            timecodeToTime(in, hour, minute, second, frame);

            /*std::stringstream ss;
            ss << std::setfill('0') << std::setw(2) << hour;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << minute;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << second;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << frame;
            return ss.str();*/

            std::string out = "00:00:00:00";
            out[0] = 48 + hour / 10;
            out[1] = 48 + hour % 10;
            out[3] = 48 + minute / 10;
            out[4] = 48 + minute % 10;
            out[6] = 48 + second / 10;
            out[7] = 48 + second % 10;
            out[9] = 48 + frame / 10;
            out[10] = 48 + frame % 10;
            return out;
        }

        void stringToTimecode(const std::string& in, uint32_t& out)
        {
            int hour = 0;
            int minute = 0;
            int second = 0;
            int frame = 0;
            const auto pieces = string::split(in, ':');
            size_t i = 0;
            if (pieces.size() != 4)
            {
                throw error::ParseError();
            }
            hour = std::stoi(pieces[i]);
            ++i;
            minute = std::stoi(pieces[i]);
            ++i;
            second = std::stoi(pieces[i]);
            ++i;
            frame = std::stoi(pieces[i]);
            out = timeToTimecode(hour, minute, second, frame);
        }
    } // namespace time
} // namespace tl

namespace opentime
{
    namespace OPENTIME_VERSION
    {
        void to_json(nlohmann::json& json, const RationalTime& value)
        {
            json = {value.value(), value.rate()};
        }

        void to_json(nlohmann::json& json, const TimeRange& value)
        {
            json = {value.start_time(), value.duration()};
        }

        void from_json(const nlohmann::json& json, RationalTime& value)
        {
            double v = 0.0;
            double rate = 0.0;
            json.at(0).get_to(v);
            json.at(1).get_to(rate);
            value = RationalTime(v, rate);
        }

        void from_json(const nlohmann::json& json, TimeRange& value)
        {
            RationalTime start;
            RationalTime duration;
            json.at(0).get_to(start);
            json.at(1).get_to(duration);
            value = TimeRange(start, duration);
        }

        std::ostream& operator<<(std::ostream& os, const RationalTime& value)
        {
            os << std::fixed << value.value() << "/" << value.rate();
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const TimeRange& value)
        {
            os << std::fixed << value.start_time().value() << "/"
               << value.duration().value() << "/" << value.duration().rate();
            return os;
        }

        std::istream& operator>>(std::istream& is, RationalTime& out)
        {
            std::string s;
            is >> s;
            auto split = tl::string::split(s, '/');
            if (split.size() != 2)
            {
                throw tl::error::ParseError();
            }
            tl::locale::SetAndRestore saved(LC_NUMERIC, "C");
            out = RationalTime(std::stof(split[0]), std::stof(split[1]));
            return is;
        }

        std::istream& operator>>(std::istream& is, TimeRange& out)
        {
            std::string s;
            is >> s;
            auto split = tl::string::split(s, '/');
            if (split.size() != 3)
            {
                throw tl::error::ParseError();
            }
            double startTime = 0.0;
            {
                std::stringstream ss(split[0]);
                ss >> startTime;
            }
            double duration = 0.0;
            {
                std::stringstream ss(split[1]);
                ss >> duration;
            }
            double rate = 0.0;
            {
                std::stringstream ss(split[2]);
                ss >> rate;
            }
            out = TimeRange(
                RationalTime(startTime, rate), RationalTime(duration, rate));
            return is;
        }
    } // namespace OPENTIME_VERSION
} // namespace opentime
