
#pragma once

#include <iostream>

namespace mrv
{
    enum class EditMode { kNone, kTimeline, kSaved, kFull };

    inline std::vector<std::string> getEditModeLabels()
    {
        std::vector< std::string > out;
        out.push_back("None");
        out.push_back("Timeline");
        out.push_back("Saved");
        out.push_back("Full");
        return out;
    }

    inline std::istream& operator>>(std::istream& o, EditMode& mode)
    {
        std::string s;
        o >> s;
        if (s == "None")
            mode = EditMode::kNone;
        else if (s == "Saved")
            mode = EditMode::kSaved;
        else if (s == "Timeline")
            mode = EditMode::kTimeline;
        else if (s == "Full")
            mode = EditMode::kFull;
        return o;
    }

    inline std::ostream& operator<<(std::ostream& o, const EditMode& mode)
    {
        switch (mode)
        {
        case EditMode::kNone:
            return o << "None";
        case EditMode::kSaved:
            return o << "Saved";
        case EditMode::kFull:
            return o << "Full";
        default:
        case EditMode::kTimeline:
            return o << "Timeline";
        }
    }
} // namespace mrv
