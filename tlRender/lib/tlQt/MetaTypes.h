// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/FileIO.h>
#include <tlCore/FontSystem.h>
#include <tlCore/Image.h>
#include <tlCore/OS.h>

#include <QMetaType>

Q_DECLARE_METATYPE(tl::audio::DataType);
Q_DECLARE_METATYPE(tl::audio::DeviceFormat);

Q_DECLARE_METATYPE(tl::log::Type);

Q_DECLARE_METATYPE(tl::file::Mode);

Q_DECLARE_METATYPE(tl::image::PixelType);
Q_DECLARE_METATYPE(tl::image::Size);
Q_DECLARE_METATYPE(tl::image::VideoLevels);

Q_DECLARE_METATYPE(tl::memory::Endian);

Q_DECLARE_METATYPE(tl::io::FileType);

Q_DECLARE_METATYPE(tl::timeline::AlphaBlend);
Q_DECLARE_METATYPE(tl::timeline::Channels);
Q_DECLARE_METATYPE(tl::timeline::CompareMode);
Q_DECLARE_METATYPE(tl::timeline::CompareTimeMode);
Q_DECLARE_METATYPE(tl::timeline::FileSequenceAudio);
Q_DECLARE_METATYPE(tl::timeline::ImageFilter);
Q_DECLARE_METATYPE(tl::timeline::InputVideoLevels);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimeUnits);
Q_DECLARE_METATYPE(tl::timeline::TimerMode);
Q_DECLARE_METATYPE(tl::timeline::Transition);
