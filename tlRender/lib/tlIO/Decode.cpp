// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Decode.h>

namespace tl
{
    namespace io
    {
        IDecode::IDecode()
        {}

        IDecode::~IDecode()
        {}

        void IDecode::_init(const std::shared_ptr<log::System>& logSystem)
        {
            _logSystem = logSystem;
        }

        double IDecode::getSpeed(const io::Info&, double defaultSpeed) const
        {
            return defaultSpeed;
        }

    }
}
