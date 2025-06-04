// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlResourceApp/App.h>

#include <tlCore/FileIO.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace resource
    {
        void App::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            BaseApp::_init(
                argv, context, "tlresource",
                "Convert a resource file to a source file.",
                {app::CmdLineValueArg<std::string>::create(
                     _input, "input", "The input resource file."),
                 app::CmdLineValueArg<std::string>::create(
                     _output, "output", "The output source code file."),
                 app::CmdLineValueArg<std::string>::create(
                     _varName, "variable name", "The resource variable name.")},
                {});
        }

        App::App() {}

        App::~App() {}

        std::shared_ptr<App> App::create(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(argv, context);
            return out;
        }

        int App::run()
        {
            if (0 == _exit)
            {
                _startTime = std::chrono::steady_clock::now();

                auto inputIO = file::FileIO::create(_input, file::Mode::Read);
                const size_t size = inputIO->getSize();
                std::vector<uint8_t> data;
                data.resize(size);
                inputIO->readU8(data.data(), size);

                auto outputIO =
                    file::FileIO::create(_output, file::Mode::Write);
                outputIO->write(
                    string::Format("const std::vector<uint8_t> {0} = {\n")
                        .arg(_varName));
                const size_t columns = 15;
                for (size_t i = 0; i < size; i += columns)
                {
                    outputIO->write("    ");
                    for (size_t j = i; j < i + columns && j < size; ++j)
                    {
                        outputIO->write(string::Format("{0}, ").arg(
                            static_cast<int>(data[j])));
                    }
                    outputIO->write("\n");
                }
                outputIO->write("};\n");

                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                _print(
                    string::Format("Seconds elapsed: {0}").arg(diff.count()));
            }
            return _exit;
        }
    } // namespace resource
} // namespace tl
