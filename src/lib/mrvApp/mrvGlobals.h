// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvLicensing.h"

namespace mrv
{
    namespace app
    {
        static bool demo_mode;
        static bool force_demo;
        
        static bool soporta_annotations;
        static bool soporta_layers;
        static bool soporta_editing;
        static bool soporta_hdr;
        static bool soporta_python;
        static bool soporta_saving;
        static bool soporta_voice;

        static std::string session_id;
        static LicenseType license_type;
    }
}
