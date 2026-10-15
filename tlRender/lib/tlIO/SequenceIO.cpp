// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SequenceIO.h>

#include <tlCore/StringFormat.h>

#include <thread>

namespace tl
{
    namespace io
    {
        TLRENDER_ENUM_IMPL(
            MissingFrames,
            "Error",
            "Hold",
            "Black",
            "Skip",
            "Gaps");

        bool isStructural(MissingFrames value)
        {
            return
                MissingFrames::Skip == value ||
                MissingFrames::Gaps == value;
        }

        SeqOptions::SeqOptions()
        {}

        bool SeqOptions::operator == (const SeqOptions& other) const
        {
            return
                defaultSpeed == other.defaultSpeed &&
                missingFrames == other.missingFrames;
        }

        bool SeqOptions::operator != (const SeqOptions& other) const
        {
            return !(*this == other);
        }

        io::Options getOptions(const SeqOptions& value)
        {
            io::Options out;
            out["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(value.defaultSpeed);
            out["SequenceIO/MissingFrames"] = to_string(value.missingFrames);
            return out;
        }

        MissingFrames getMissingFrames(const io::Options& value)
        {
            MissingFrames out = SeqOptions().missingFrames;
            if (const auto i = value.find("SequenceIO/MissingFrames"); i != value.end())
            {
                from_string(i->second, out);
            }
            return out;
        }

        void to_json(nlohmann::json& json, const SeqOptions& value)
        {
            json["DefaultSpeed"] = value.defaultSpeed;
            json["MissingFrames"] = to_string(value.missingFrames);
        }

        void from_json(const nlohmann::json& json, SeqOptions& value)
        {
            json.at("DefaultSpeed").get_to(value.defaultSpeed);
            json.at("MissingFrames").get_to(value.missingFrames);
        }
    }
}
