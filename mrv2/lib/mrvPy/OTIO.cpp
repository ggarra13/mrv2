// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvI8N.h"

namespace py = pybind11;
using namespace pybind11::literals;

using namespace opentime;

namespace
{
    struct ErrorStatusConverter
    {
        operator ErrorStatus*() { return &error_status; }

        ~ErrorStatusConverter() noexcept(false)
        {
            namespace py = pybind11;
            if (is_error(error_status))
            {
                throw py::value_error(error_status.details);
            }
        }

        ErrorStatus error_status;
    };

    IsDropFrameRate df_enum_converter(py::object& df)
    {
        if (df.is(py::none()))
        {
            return IsDropFrameRate::InferFromRate;
        }
        else if (py::cast<bool>(py::bool_(df)))
        {
            return IsDropFrameRate::ForceYes;
        }
        else
        {
            return IsDropFrameRate::ForceNo;
        }
    }
} // namespace

std::string opentime_python_str(RationalTime rt)
{
    return tl::string::Format("RationalTime({0}, {1})")
        .arg(rt.value())
        .arg(rt.rate());
}

std::string opentime_python_repr(RationalTime rt)
{
    return tl::string::Format("otio.opentime.RationalTime(value={0}, rate={1})")
        .arg(rt.value())
        .arg(rt.rate());
}

RationalTime _type_checked(py::object const& rhs, char const* op)
{
    try
    {
        return py::cast<RationalTime>(rhs);
    }
    catch (...)
    {
        std::string rhs_type =
            py::cast<std::string>(py::type::of(rhs).attr("__name__"));
        throw py::type_error(
            tl::string::Format(_("unsupported operand type(s) for {0}: "
                                 "RationalTime and {1}"))
                .arg(op)
                .arg(rhs_type));
    }
}

void opentime_rationalTime_bindings(py::module m)
{
    py::class_<RationalTime>(m, "RationalTime", _(R"docstring(
The RationalTime class represents a measure of time of :math:`rt.value/rt.rate` seconds.
It can be rescaled into another :class:`~RationalTime`'s rate.
)docstring"))
        .def(py::init<double, double>(), "value"_a = 0, "rate"_a = 1)
        .def("is_invalid_time", &RationalTime::is_invalid_time, _(R"docstring(
Returns true if the time is invalid. The time is considered invalid if the value or the rate are a NaN value
or if the rate is less than or equal to zero.
)docstring"))
        .def_property_readonly("value", &RationalTime::value)
        .def_property_readonly("rate", &RationalTime::rate)
        .def(
            "rescaled_to",
            (RationalTime(RationalTime::*)(double) const) &
                RationalTime::rescaled_to,
            "new_rate"_a,
            _(R"docstring(Returns the time value for time converted to new_rate.)docstring"))
        .def(
            "rescaled_to",
            (RationalTime(RationalTime::*)(RationalTime) const) &
                RationalTime::rescaled_to,
            "other"_a,
            _(R"docstring(Returns the time for time converted to new_rate.)docstring"))
        .def(
            "value_rescaled_to",
            (double(RationalTime::*)(double) const) &
                RationalTime::value_rescaled_to,
            "new_rate"_a,
            _(R"docstring(Returns the time value for "self" converted to new_rate.)docstring"))
        .def(
            "value_rescaled_to",
            (double(RationalTime::*)(RationalTime) const) &
                RationalTime::value_rescaled_to,
            "other"_a)
        .def(
            "almost_equal", &RationalTime::almost_equal, "other"_a,
            "delta"_a = 0)
        .def("__copy__", [](RationalTime rt) { return rt; })
        .def(
            "__deepcopy__", [](RationalTime rt, py::object) { return rt; },
            "copier"_a = py::none())
        .def_static(
            "duration_from_start_end_time",
            &RationalTime::duration_from_start_end_time, "start_time"_a,
            "end_time_exclusive"_a, _(R"docstring(
Compute the duration of samples from first to last (excluding last). This is not the same as distance.

For example, the duration of a clip from frame 10 to frame 15 is 5 frames. Result will be in the rate of start_time.
)docstring"))
        .def_static(
            "duration_from_start_end_time_inclusive",
            &RationalTime::duration_from_start_end_time_inclusive,
            "start_time"_a, "end_time_inclusive"_a, _(R"docstring(
Compute the duration of samples from first to last (including last). This is not the same as distance.

For example, the duration of a clip from frame 10 to frame 15 is 6 frames. Result will be in the rate of start_time.
)docstring"))
        .def_static(
            "is_valid_timecode_rate", &RationalTime::is_valid_timecode_rate,
            "rate"_a,
            _("Returns true if the rate is valid for use with timecode."))
        .def_static(
            "nearest_valid_timecode_rate",
            &RationalTime::nearest_valid_timecode_rate, "rate"_a,
            _("Returns the first valid timecode rate that has the least "
              "difference from the given value."))
        .def_static(
            "from_frames", &RationalTime::from_frames, "frame"_a, "rate"_a,
            _("Turn a frame number and rate into a :class:`~RationalTime` "
              "object."))
        .def_static(
            "from_seconds",
            static_cast<RationalTime (*)(double, double)>(
                &RationalTime::from_seconds),
            "seconds"_a, "rate"_a)
        .def_static(
            "from_seconds",
            static_cast<RationalTime (*)(double)>(&RationalTime::from_seconds),
            "seconds"_a)
        .def(
            "to_frames",
            (int(RationalTime::*)() const) & RationalTime::to_frames,
            _("Returns the frame number based on the current rate."))
        .def(
            "to_frames",
            (int(RationalTime::*)(double) const) & RationalTime::to_frames,
            "rate"_a, _("Returns the frame number based on the given rate."))
        .def("to_seconds", &RationalTime::to_seconds)
        .def(
            "to_timecode",
            [](RationalTime rt, double rate, py::object drop_frame)
            {
                return rt.to_timecode(
                    rate, df_enum_converter(drop_frame),
                    ErrorStatusConverter());
            },
            "rate"_a, "drop_frame"_a,
            _("Convert to timecode (``HH:MM:SS;FRAME``)"))
        .def(
            "to_timecode",
            [](RationalTime rt, double rate)
            {
                return rt.to_timecode(
                    rate, IsDropFrameRate::InferFromRate,
                    ErrorStatusConverter());
            },
            "rate"_a)
        .def(
            "to_timecode",
            [](RationalTime rt)
            {
                return rt.to_timecode(
                    rt.rate(), IsDropFrameRate::InferFromRate,
                    ErrorStatusConverter());
            })
        .def("to_time_string", &RationalTime::to_time_string)
        .def_static(
            "from_timecode",
            [](std::string s, double rate) {
                return RationalTime::from_timecode(
                    s, rate, ErrorStatusConverter());
            },
            "timecode"_a, "rate"_a,
            _("Convert a timecode string (``HH:MM:SS;FRAME``) into a "
              ":class:`~RationalTime`."))
        .def_static(
            "from_time_string",
            [](std::string s, double rate) {
                return RationalTime::from_time_string(
                    s, rate, ErrorStatusConverter());
            },
            "time_string"_a, "rate"_a,
            _("Convert a time with microseconds string (``HH:MM:ss`` where "
              "``ss`` is an integer or a decimal number) into a "
              ":class:`~RationalTime`."))
        .def("__str__", &opentime_python_str)
        .def("__repr__", &opentime_python_repr)
        .def(-py::self)
        .def(
            "__lt__", [](RationalTime lhs, py::object const& rhs)
            { return lhs < _type_checked(rhs, "<"); })
        .def(
            "__gt__", [](RationalTime lhs, py::object const& rhs)
            { return lhs > _type_checked(rhs, ">"); })
        .def(
            "__le__", [](RationalTime lhs, py::object const& rhs)
            { return lhs <= _type_checked(rhs, "<="); })
        .def(
            "__ge__", [](RationalTime lhs, py::object const& rhs)
            { return lhs >= _type_checked(rhs, ">="); })
        .def(
            "__eq__", [](RationalTime lhs, py::object const& rhs)
            { return lhs == _type_checked(rhs, "=="); })
        .def(
            "__ne__", [](RationalTime lhs, py::object const& rhs)
            { return lhs != _type_checked(rhs, "!="); })
        .def(py::self - py::self)
        .def(py::self + py::self)
        // The simple "py::self += py::self" returns the original,
        // which is not what we want here: we need this to return a new copy
        // to avoid mutating any additional references, since this class has
        // complete value semantics.

        .def(
            "__iadd__",
            [](RationalTime lhs, RationalTime rhs) { return lhs += rhs; });

    py::module test =
        m.def_submodule("_testing", "Module for regression tests");
    test.def(
        "add_many",
        [](RationalTime step_time, int final_frame_number)
        {
            RationalTime sum = step_time;
            for (int i = 1; i < final_frame_number; i++)
            {
                sum += step_time;
            }
            return sum;
        });
}

void opentime_timeRange_bindings(py::module m)
{
    py::class_<TimeRange>(m, "TimeRange", _(R"docstring(
The TimeRange class represents a range in time. It encodes the start time and the duration,
meaning that :meth:`end_time_inclusive` (last portion of a sample in the time range) and
:meth:`end_time_exclusive` can be computed.
)docstring"))
        // matches the python constructor behavior
        .def(
            py::init(
                [](RationalTime* start_time, RationalTime* duration)
                {
                    if (start_time == nullptr && duration == nullptr)
                    {
                        return TimeRange();
                    }
                    else if (start_time == nullptr)
                    {
                        return TimeRange(
                            RationalTime(0.0, duration->rate()), *duration);
                    }
                    // duration == nullptr
                    else if (duration == nullptr)
                    {
                        return TimeRange(
                            *start_time, RationalTime(0.0, start_time->rate()));
                    }
                    else
                    {
                        return TimeRange(*start_time, *duration);
                    }
                }),
            "start_time"_a = nullptr, "duration"_a = nullptr)
        .def_property_readonly("start_time", &TimeRange::start_time)
        .def_property_readonly("duration", &TimeRange::duration)
        .def("end_time_inclusive", &TimeRange::end_time_inclusive,
             _(R"docstring(
The time of the last sample containing data in the time range.

If the time range starts at (0, 24) with duration (10, 24), this will be
(9, 24)

If the time range starts at (0, 24) with duration (10.5, 24):
(10, 24)

In other words, the last frame with data, even if the last frame is fractional.
)docstring"))
        .def("end_time_exclusive", &TimeRange::end_time_exclusive,
             _(R"docstring(
Time of the first sample outside the time range.

If start frame is 10 and duration is 5, then end_time_exclusive is 15,
because the last time with data in this range is 14.

If start frame is 10 and duration is 5.5, then end_time_exclusive is
15.5, because the last time with data in this range is 15.
)docstring"))
        .def(
            "duration_extended_by", &TimeRange::duration_extended_by, "other"_a)
        .def(
            "extended_by", &TimeRange::extended_by, "other"_a,
            _("Construct a new :class:`~TimeRange` that is this one extended "
              "by other."))
        .def(
            "clamped",
            (RationalTime(TimeRange::*)(RationalTime) const) &
                TimeRange::clamped,
            "other"_a, _(R"docstring(
Clamp 'other' (:class:`~RationalTime`) according to
:attr:`start_time`/:attr:`end_time_exclusive` and bound arguments.
)docstring"))
        .def(
            "clamped",
            (TimeRange(TimeRange::*)(TimeRange) const) & TimeRange::clamped,
            "other"_a, _(R"docstring(
Clamp 'other' (:class:`~TimeRange`) according to
:attr:`start_time`/:attr:`end_time_exclusive` and bound arguments.
)docstring"))
        .def(
            "contains",
            (bool(TimeRange::*)(RationalTime) const) & TimeRange::contains,
            "other"_a, _(R"docstring(
The start of `this` precedes `other`.
`other` precedes the end of `this`.
::

         other
           |
           *
   [      this      ]

)docstring"))
        .def(
            "contains",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::contains,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` precedes start of `other`.
The end of `this` antecedes end of `other`.
::

        [ other ]
   [      this      ]

The converse would be ``other.contains(this)``
)docstring"))
        .def(
            "overlaps",
            (bool(TimeRange::*)(RationalTime) const) & TimeRange::overlaps,
            "other"_a, _(R"docstring(
`this` contains `other`.
::

        other
         |
         *
   [    this    ]

)docstring"))
        .def(
            "overlaps",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::overlaps,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` strictly precedes end of `other` by a value >= `epsilon_s`.
The end of `this` strictly antecedes start of `other` by a value >= `epsilon_s`.
::

   [ this ]
       [ other ]

The converse would be ``other.overlaps(this)``
)docstring"))
        .def(
            "before",
            (bool(TimeRange::*)(RationalTime, double) const) &
                TimeRange::before,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The end of `this` strictly precedes `other` by a value >= `epsilon_s`.
::

             other
               |
   [ this ]    *

)docstring"))
        .def(
            "before",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::before,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The end of `this` strictly equals the start of `other` and
the start of `this` strictly equals the end of `other`.
::

   [this][other]

The converse would be ``other.meets(this)``
)docstring"))
        .def(
            "meets",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::meets,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The end of `this` strictly equals the start of `other` and
the start of `this` strictly equals the end of `other`.
::

   [this][other]

The converse would be ``other.meets(this)``
)docstring"))
        .def(
            "begins",
            (bool(TimeRange::*)(RationalTime, double) const) &
                TimeRange::begins,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` strictly equals `other`.
::

   other
     |
     *
     [ this ]

)docstring"))
        .def(
            "begins",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::begins,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` strictly equals the start of `other`.
The end of `this` strictly precedes the end of `other` by a value >= `epsilon_s`.
::

   [ this ]
   [    other    ]

The converse would be ``other.begins(this)``
)docstring"))
        .def(
            "finishes",
            (bool(TimeRange::*)(RationalTime, double) const) &
                TimeRange::finishes,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The end of `this` strictly equals `other`.
::

        other
          |
          *
   [ this ]

)docstring"))
        .def(
            "finishes",
            (bool(TimeRange::*)(TimeRange, double) const) & TimeRange::finishes,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` strictly antecedes the start of `other` by a value >= `epsilon_s`.
The end of `this` strictly equals the end of `other`.
::

           [ this ]
   [     other    ]

The converse would be ``other.finishes(this)``
)docstring"))
        .def(
            "intersects",
            (bool(TimeRange::*)(TimeRange, double) const) &
                TimeRange::intersects,
            "other"_a, "epsilon_s"_a = opentime::DEFAULT_EPSILON_s,
            _(R"docstring(
The start of `this` precedes or equals the end of `other` by a value >= `epsilon_s`.
The end of `this` antecedes or equals the start of `other` by a value >= `epsilon_s`.
::

   [    this    ]           OR      [    other    ]
        [     other    ]                    [     this    ]

The converse would be ``other.finishes(this)``
)docstring"))
        .def("__copy__", [](TimeRange tr) { return tr; })
        .def("__deepcopy__", [](TimeRange tr, py::object memo) { return tr; })
        .def_static(
            "range_from_start_end_time", &TimeRange::range_from_start_end_time,
            "start_time"_a, "end_time_exclusive"_a, _(R"docstring(
Creates a :class:`~TimeRange` from start and end :class:`~RationalTime`\s (exclusive).

For example, if start_time is 1 and end_time is 10, the returned will have a duration of 9.
)docstring"))
        .def_static(
            "range_from_start_end_time_inclusive",
            &TimeRange::range_from_start_end_time_inclusive, "start_time"_a,
            "end_time_inclusive"_a, _(R"docstring(
Creates a :class:`~TimeRange` from start and end :class:`~RationalTime`\s (inclusive).

For example, if start_time is 1 and end_time is 10, the returned will have a duration of 10.
)docstring"))
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(
            "__str__",
            [](TimeRange tr)
            {
                std::string out = tl::string::Format("TimeRange({0}, {1})")
                    .arg(opentime_python_str(tr.start_time()))
                    .arg(opentime_python_str(tr.duration()));
                return out;
            })
        .def(
            "__repr__",
            [](TimeRange tr)
            {
                std::string out = tl::string::Format(
                           "mrv2.TimeRange(start_time={0}, duration={1})")
                    .arg(opentime_python_str(tr.start_time()))
                    .arg(opentime_python_str(tr.duration()));
                return out;
            });
}

#include <pybind11/pybind11.h>

namespace py = pybind11;

void mrv2_otio(py::module& m)
{
    opentime_rationalTime_bindings(m);
    opentime_timeRange_bindings(m);
}
