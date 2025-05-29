// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQt/Init.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <tlTimeline/Init.h>

#include <tlCore/Context.h>
#include <tlCore/Mesh.h>

#include <QSurfaceFormat>

namespace tl
{
    namespace qt
    {
        void init(
            DefaultSurfaceFormat defaultSurfaceFormat,
            const std::shared_ptr<system::Context>& context)
        {
            timeline::init(context);
            if (!context->getSystem<System>())
            {
                context->addSystem(
                    System::create(defaultSurfaceFormat, context));
            }
        }

        void System::_init(
            DefaultSurfaceFormat defaultSurfaceFormat,
            const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::qt::System", context);

            qRegisterMetaType<otime::RationalTime>("otime::RationalTime");
            qRegisterMetaType<otime::TimeRange>("otime::TimeRange");
            qRegisterMetaType<std::vector<otime::TimeRange> >(
                "std::vector<otime::TimeRange>");

            qRegisterMetaType<audio::DataType>("tl::audio::DataType");
            qRegisterMetaType<audio::DeviceFormat>("tl::audio::DeviceFormat");
            qRegisterMetaType<audio::Device>("tl::audio::Device");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<audio::DataType>();
            QMetaType::registerComparators<audio::DeviceFormat>();
#endif // QT_VERSION

            qRegisterMetaType<log::Item>("tl::log::Item");
            qRegisterMetaType<log::Type>("tl::log::Type");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<log::Type>();
#endif // QT_VERSION

            qRegisterMetaType<file::Mode>("tl::file::Mode");
            qRegisterMetaType<file::PathOptions>("tl::file::PathOptions");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<file::Mode>();
#endif // QT_VERSION

            qRegisterMetaType<geom::Triangle2>("tl::geom::Triangle2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::Triangle3");
            qRegisterMetaType<geom::Triangle2>("tl::geom::TriangleMesh2");
            qRegisterMetaType<geom::Triangle3>("tl::geom::TriangleMesh3");
            qRegisterMetaType<geom::Vertex2>("tl::geom::Vertex2");
            qRegisterMetaType<geom::Vertex3>("tl::geom::Vertex3");

            qRegisterMetaType<image::FontInfo>("tl::image::FontInfo");
            qRegisterMetaType<image::FontMetrics>("tl::image::FontMetrics");
            qRegisterMetaType<image::GlyphInfo>("tl::image::GlyphInfo");
            qRegisterMetaType<image::Glyph>("tl::image::Glyph");
            qRegisterMetaType<image::PixelType>("tl::image::PixelType");
            qRegisterMetaType<image::Size>("tl::image::Size");
            qRegisterMetaType<image::VideoLevels>("tl::image::VideoLevels");

            qRegisterMetaType<math::Size2i>("tl::math::Size2i");

            qRegisterMetaType<memory::Endian>("tl::memory::Endian");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<memory::Endian>();
#endif // QT_VERSION

            qRegisterMetaType<observer::CallbackAction>(
                "tl::observer::CallbackAction");

            qRegisterMetaType<os::SystemInfo>("tl::os::SystemInfo");

            qRegisterMetaType<io::FileType>("tl::io::FileType");
            qRegisterMetaType<io::Info>("tl::io::Info");
            qRegisterMetaType<io::VideoData>("tl::io::VideoData");
            qRegisterMetaType<io::AudioData>("tl::io::AudioData");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<io::FileType>();
#endif // QT_VERSION

            qRegisterMetaType<timeline::AlphaBlend>("tl::timeline::AlphaBlend");
            qRegisterMetaType<timeline::AudioData>("tl::timeline::AudioData");
            qRegisterMetaType<timeline::AudioLayer>("tl::timeline::AudioLayer");
            qRegisterMetaType<timeline::Channels>("tl::timeline::Channels");
            qRegisterMetaType<timeline::Color>("tl::timeline::Color");
            qRegisterMetaType<timeline::CompareMode>(
                "tl::timeline::CompareMode");
            qRegisterMetaType<timeline::CompareTimeMode>(
                "tl::timeline::CompareTimeMode");
            qRegisterMetaType<timeline::CompareOptions>(
                "tl::timeline::CompareOptions");
            qRegisterMetaType<timeline::EXRDisplay>("tl::timeline::EXRDisplay");
            qRegisterMetaType<timeline::FileSequenceAudio>(
                "tl::timeline::FileSequenceAudio");
            qRegisterMetaType<timeline::ImageFilter>(
                "tl::timeline::ImageFilter");
            qRegisterMetaType<timeline::ImageOptions>(
                "tl::timeline::ImageOptions");
            qRegisterMetaType<timeline::InputVideoLevels>(
                "tl::timeline::InputVideoLevels");
            qRegisterMetaType<timeline::LUTOptions>("tl::timeline::LUTOptions");
            qRegisterMetaType<timeline::Levels>("tl::timeline::Levels");
            qRegisterMetaType<timeline::Loop>("tl::timeline::Loop");
            qRegisterMetaType<timeline::OCIOOptions>(
                "tl::timeline::OCIOOptions");
            qRegisterMetaType<timeline::Options>("tl::timeline::Options");
            qRegisterMetaType<timeline::Playback>("tl::timeline::Playback");
            qRegisterMetaType<timeline::PlayerCacheInfo>(
                "tl::timeline::PlayerCacheInfo");
            qRegisterMetaType<timeline::PlayerCacheOptions>(
                "tl::timeline::PlayerCacheOptions");
            qRegisterMetaType<timeline::PlayerOptions>(
                "tl::timeline::PlayerOptions");
            qRegisterMetaType<timeline::TimeAction>("tl::timeline::TimeAction");
            qRegisterMetaType<timeline::TimeUnits>("tl::timeline::TimeUnits");
            qRegisterMetaType<timeline::TimerMode>("tl::timeline::TimerMode");
            qRegisterMetaType<timeline::Transition>("tl::timeline::Transition");
            qRegisterMetaType<timeline::VideoData>("tl::timeline::VideoData");
            qRegisterMetaType<timeline::VideoLayer>("tl::timeline::VideoLayer");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<timeline::AlphaBlend>();
            QMetaType::registerComparators<timeline::Channels>();
            QMetaType::registerComparators<timeline::CompareMode>();
            QMetaType::registerComparators<timeline::CompareTimeMode>();
            QMetaType::registerComparators<timeline::FileSequenceAudio>();
            QMetaType::registerComparators<timeline::ImageFilter>();
            QMetaType::registerComparators<timeline::InputVideoLevels>();
            QMetaType::registerComparators<timeline::Loop>();
            QMetaType::registerComparators<timeline::Playback>();
            QMetaType::registerComparators<timeline::TimeAction>();
            QMetaType::registerComparators<timeline::TimeUnits>();
            QMetaType::registerComparators<timeline::TimerMode>();
            QMetaType::registerComparators<timeline::Transition>();
#endif // QT_VERSION

            switch (defaultSurfaceFormat)
            {
            case DefaultSurfaceFormat::OpenGL_4_1_CoreProfile:
            {
                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                QSurfaceFormat::setDefaultFormat(surfaceFormat);
                break;
            }
            default:
                break;
            }
        }

        System::System() {}

        System::~System() {}

        std::shared_ptr<System> System::create(
            DefaultSurfaceFormat defaultSurfaceFormat,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(defaultSurfaceFormat, context);
            return out;
        }
    } // namespace qt
} // namespace tl
