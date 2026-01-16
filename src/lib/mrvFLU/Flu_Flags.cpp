// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvApp.h"

namespace mrv
{
    bool App::demo_mode = true;
    bool App::force_demo = false;

    std::string App::session_id = "";
    LicenseType App::license_type = LicenseType::kDemo;

    bool App::soporta_saving = true;
    bool App::soporta_layers = true;
    bool App::soporta_annotations = false;
    bool App::soporta_editing = false;
    bool App::soporta_hdr = true;
    bool App::soporta_python = false;
    bool App::soporta_voice = false;

}  // namespace mrv
