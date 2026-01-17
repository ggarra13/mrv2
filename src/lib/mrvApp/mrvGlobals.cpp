// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvLicensing.h"

namespace mrv
{
    namespace app
    {
        bool demo_mode = true;
        bool force_demo = false;

        std::string session_id = "";
        LicenseType license_type = LicenseType::kDemo;
        
        bool soporta_annotations = false;
        bool soporta_layers = true;
        bool soporta_editing = false;
        bool soporta_hdr = true;
        bool soporta_python = false;
        bool soporta_saving = true;
        bool soporta_voice = false;

    }
}
