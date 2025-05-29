// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
#    include <tlQtTest/TimeObjectTest.h>
#    include <tlQt/Init.h>
#endif // TLRENDER_QT5 || TLRENDER_QT6

#include <tlGLTest/GLFWTest.h>
#include <tlGLTest/MeshTest.h>
#include <tlGLTest/OffscreenBufferTest.h>
#include <tlGLTest/ShaderTest.h>
#include <tlGLTest/TextureTest.h>
#include <tlGL/Init.h>

#include <tlAppTest/AppTest.h>
#include <tlAppTest/CmdLineTest.h>

#include <tlTimelineTest/CompareOptionsTest.h>
#include <tlTimelineTest/DisplayOptionsTest.h>
#include <tlTimelineTest/EditTest.h>
#include <tlTimelineTest/IRenderTest.h>
#include <tlTimelineTest/ImageOptionsTest.h>
#include <tlTimelineTest/LUTOptionsTest.h>
#include <tlTimelineTest/MemoryReferenceTest.h>
#include <tlTimelineTest/OCIOOptionsTest.h>
#include <tlTimelineTest/PlayerOptionsTest.h>
#include <tlTimelineTest/PlayerTest.h>
#include <tlTimelineTest/TimelineTest.h>
#include <tlTimelineTest/UtilTest.h>

#include <tlIOTest/CineonTest.h>
#include <tlIOTest/DPXTest.h>
#include <tlIOTest/IOTest.h>
#include <tlIOTest/PPMTest.h>
#include <tlIOTest/SGITest.h>
#if defined(TLRENDER_FFMPEG)
#    include <tlIOTest/FFmpegTest.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
#    include <tlIOTest/JPEGTest.h>
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
#    include <tlIOTest/OpenEXRTest.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
#    include <tlIOTest/PNGTest.h>
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
#    include <tlIOTest/TIFFTest.h>
#endif // TLRENDER_TIFF
#if defined(TLRENDER_STB)
#    include <tlIOTest/STBTest.h>
#endif // TLRENDER_STB

#include <tlCoreTest/AudioTest.h>
#include <tlCoreTest/BoxTest.h>
#include <tlCoreTest/ColorTest.h>
#include <tlCoreTest/ContextTest.h>
#include <tlCoreTest/ErrorTest.h>
#include <tlCoreTest/FileIOTest.h>
#include <tlCoreTest/FileInfoTest.h>
#include <tlCoreTest/FileTest.h>
#include <tlCoreTest/FontSystemTest.h>
#include <tlCoreTest/HDRTest.h>
#include <tlCoreTest/ImageTest.h>
#include <tlCoreTest/LRUCacheTest.h>
#include <tlCoreTest/ListObserverTest.h>
#include <tlCoreTest/LogSystemTest.h>
#include <tlCoreTest/MapObserverTest.h>
#include <tlCoreTest/MathTest.h>
#include <tlCoreTest/MatrixTest.h>
#include <tlCoreTest/MemoryTest.h>
#include <tlCoreTest/MeshTest.h>
#include <tlCoreTest/OSTest.h>
#include <tlCoreTest/PathTest.h>
#include <tlCoreTest/RangeTest.h>
#include <tlCoreTest/SizeTest.h>
#include <tlCoreTest/StringTest.h>
#include <tlCoreTest/StringFormatTest.h>
#include <tlCoreTest/TimeTest.h>
#include <tlCoreTest/ValueObserverTest.h>
#include <tlCoreTest/VectorTest.h>

#include <tlTimeline/Init.h>

#include <tlCore/Context.h>

#include <iostream>
#include <vector>

using namespace tl;
using namespace tl::tests;

void coreTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
    tests.push_back(core_tests::AudioTest::create(context));
    tests.push_back(core_tests::BoxTest::create(context));
    tests.push_back(core_tests::ColorTest::create(context));
    tests.push_back(core_tests::ContextTest::create(context));
    tests.push_back(core_tests::ErrorTest::create(context));
    tests.push_back(core_tests::FileIOTest::create(context));
    tests.push_back(core_tests::FileInfoTest::create(context));
    tests.push_back(core_tests::FileTest::create(context));
    tests.push_back(core_tests::FontSystemTest::create(context));
    tests.push_back(core_tests::HDRTest::create(context));
    tests.push_back(core_tests::ImageTest::create(context));
    tests.push_back(core_tests::LRUCacheTest::create(context));
    tests.push_back(core_tests::ListObserverTest::create(context));
    tests.push_back(core_tests::LogSystemTest::create(context));
    tests.push_back(core_tests::MapObserverTest::create(context));
    tests.push_back(core_tests::MathTest::create(context));
    tests.push_back(core_tests::MatrixTest::create(context));
    tests.push_back(core_tests::MemoryTest::create(context));
    tests.push_back(core_tests::MeshTest::create(context));
    tests.push_back(core_tests::OSTest::create(context));
    tests.push_back(core_tests::PathTest::create(context));
    tests.push_back(core_tests::RangeTest::create(context));
    tests.push_back(core_tests::SizeTest::create(context));
    tests.push_back(core_tests::StringTest::create(context));
    tests.push_back(core_tests::StringFormatTest::create(context));
    tests.push_back(core_tests::TimeTest::create(context));
    tests.push_back(core_tests::ValueObserverTest::create(context));
    tests.push_back(core_tests::VectorTest::create(context));
}

void glTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
#if defined(TLRENDER_GLFW)
    tests.push_back(gl_tests::GLFWTest::create(context));
    tests.push_back(gl_tests::MeshTest::create(context));
    tests.push_back(gl_tests::OffscreenBufferTest::create(context));
    tests.push_back(gl_tests::ShaderTest::create(context));
    tests.push_back(gl_tests::TextureTest::create(context));
#endif // TLRENDER_GLFW
}

void ioTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
    tests.push_back(io_tests::CineonTest::create(context));
    tests.push_back(io_tests::DPXTest::create(context));
    tests.push_back(io_tests::IOTest::create(context));
    tests.push_back(io_tests::PPMTest::create(context));
    tests.push_back(io_tests::SGITest::create(context));
#if defined(TLRENDER_FFMPEG)
    tests.push_back(io_tests::FFmpegTest::create(context));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
    tests.push_back(io_tests::JPEGTest::create(context));
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
    tests.push_back(io_tests::OpenEXRTest::create(context));
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
    tests.push_back(io_tests::PNGTest::create(context));
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
    tests.push_back(io_tests::TIFFTest::create(context));
#endif // TLRENDER_TIFF
#if defined(TLRENDER_STB)
    tests.push_back(io_tests::STBTest::create(context));
#endif // TLRENDER_STB
}

void timelineTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
    tests.push_back(timeline_tests::CompareOptionsTest::create(context));
    tests.push_back(timeline_tests::DisplayOptionsTest::create(context));
    tests.push_back(timeline_tests::EditTest::create(context));
    tests.push_back(timeline_tests::IRenderTest::create(context));
    tests.push_back(timeline_tests::ImageOptionsTest::create(context));
    tests.push_back(timeline_tests::LUTOptionsTest::create(context));
    tests.push_back(timeline_tests::MemoryReferenceTest::create(context));
    tests.push_back(timeline_tests::OCIOOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerTest::create(context));
    tests.push_back(timeline_tests::TimelineTest::create(context));
    tests.push_back(timeline_tests::UtilTest::create(context));
}

void appTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
    tests.push_back(app_tests::AppTest::create(context));
    tests.push_back(app_tests::CmdLineTest::create(context));
}

void qtTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<system::Context>& context)
{
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    tests.push_back(qt_tests::TimeObjectTest::create(context));
#endif // TLRENDER_QT5 || TLRENDER_QT6
}

int main(int argc, char* argv[])
{
    auto context = system::Context::create();
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    qt::init(qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile, context);
#else  // TLRENDER_QT5 || TLRENDER_QT6
    timeline::init(context);
#endif // TLRENDER_QT5 || TLRENDER_QT6

    auto logObserver = observer::ListObserver<log::Item>::create(
        context->getSystem<log::System>()->observeLog(),
        [](const std::vector<log::Item>& value)
        {
            const size_t options =
                static_cast<size_t>(log::StringConvert::Time) |
                static_cast<size_t>(log::StringConvert::Prefix);
            for (const auto& i : value)
            {
                std::cout << "[LOG] " << toString(i, options) << std::endl;
            }
        },
        observer::CallbackAction::Suppress);

    context->tick();

    std::vector<std::shared_ptr<tests::ITest> > tests;
    // tests.push_back(core_tests::PathTest::create(context));
    // tests.push_back(timeline_tests::TimelineTest::create(context));
    coreTests(tests, context);
    glTests(tests, context);
    ioTests(tests, context);
    timelineTests(tests, context);
    appTests(tests, context);
    qtTests(tests, context);

    for (const auto& test : tests)
    {
        std::cout << "Running test: " << test->getName() << std::endl;
        test->run();
        context->tick();
    }

    std::cout << "Finished tests" << std::endl;
    return 0;
}
