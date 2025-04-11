// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#define LOG_INFO(x)                                                            \
    if (auto logSystem = _logSystem.lock())                                    \
    {                                                                          \
        logSystem->print(                                                      \
            "tl::io::ffmpeg::Plugin", x, log::Type::Message, kModule);         \
    }
#define LOG_ERROR(x)                                                           \
    if (auto logSystem = _logSystem.lock())                                    \
    {                                                                          \
        logSystem->print(                                                      \
            "tl::io::ffmpeg::Plugin", x, log::Type::Error, kModule);           \
    }
#define LOG_WARNING(x)                                                         \
    if (auto logSystem = _logSystem.lock())                                    \
    {                                                                          \
        logSystem->print(                                                      \
            "tl::io::ffmpeg::Plugin", x, log::Type::Warning, kModule);         \
    }
#define LOG_STATUS(x)                                                          \
    if (auto logSystem = _logSystem.lock())                                    \
    {                                                                          \
        logSystem->print(                                                      \
            "tl::io::ffmpeg::Plugin", x, log::Type::Status, kModule);          \
    }
