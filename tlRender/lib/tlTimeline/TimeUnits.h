// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Time.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace timeline
    {
        //! Time units.
        enum class TimeUnits {
            Frames,
            Seconds,
            Timecode,

            Count,
            First = Frames
        };
        TLRENDER_ENUM(TimeUnits);
        TLRENDER_ENUM_SERIALIZE(TimeUnits);

        //! Convert a time value to text.
        std::string timeToText(const otime::RationalTime&, timeline::TimeUnits);

        //! Convert text to a time value.
        otime::RationalTime textToTime(
            const std::string& text, double rate, timeline::TimeUnits units,
            otime::ErrorStatus* error = nullptr);

        //! Get a time units format string.
        std::string formatString(timeline::TimeUnits);

        //! Get a time units validator regular expression.
        std::string validator(timeline::TimeUnits);

        //! Base class for time units models.
        class ITimeUnitsModel
            : public std::enable_shared_from_this<ITimeUnitsModel>
        {
            TLRENDER_NON_COPYABLE(ITimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ITimeUnitsModel();

        public:
            virtual ~ITimeUnitsModel() = 0;

            //! Observer when the time units are changed.
            std::shared_ptr<observer::IValue<bool> >
            observeTimeUnitsChanged() const;

            //! Get a time label in the current time units.
            virtual std::string getLabel(const otime::RationalTime&) const = 0;

        protected:
            std::shared_ptr<observer::Value<bool> > _timeUnitsChanged;
        };

        //! Time units model.
        class TimeUnitsModel : public ITimeUnitsModel
        {
            TLRENDER_NON_COPYABLE(TimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            TimeUnitsModel();

        public:
            virtual ~TimeUnitsModel();

            //! Create a new model.
            static std::shared_ptr<TimeUnitsModel>
            create(const std::shared_ptr<system::Context>&);

            //! Get the time units.
            TimeUnits getTimeUnits() const;

            //! Observer the time units.
            std::shared_ptr<observer::IValue<TimeUnits> >
            observeTimeUnits() const;

            //! Set the time units.
            void setTimeUnits(TimeUnits);

            std::string getLabel(const otime::RationalTime&) const override;

            TLRENDER_PRIVATE();
        };
    } // namespace timeline
} // namespace tl
