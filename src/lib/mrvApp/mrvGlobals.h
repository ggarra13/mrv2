// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvLicensing.h"

namespace mrv
{
    namespace app
    {
        extern bool demo_mode;
        extern bool force_demo;
        
        extern bool soporta_annotations;
        extern bool soporta_layers;
        extern bool soporta_editing;
        extern bool soporta_hdr;
        extern bool soporta_python;
        extern bool soporta_saving;
        extern bool soporta_voice;

        extern std::string session_id;
        extern LicenseType license_type;
    }
}
