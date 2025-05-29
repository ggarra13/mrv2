// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileInfoTest.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/FileIO.h>
#include <tlCore/FileInfo.h>

#include <cstdio>
#include <sstream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileInfoTest::FileInfoTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FileInfoTest", context)
        {
        }

        std::shared_ptr<FileInfoTest>
        FileInfoTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FileInfoTest>(new FileInfoTest(context));
        }

        void FileInfoTest::run()
        {
            _enums();
            _ctors();
            _sequence();
            _list();
        }

        void FileInfoTest::_enums()
        {
            _enum<Type>("Type", getTypeEnums);
            _enum<ListSort>("ListSort", getListSortEnums);
        }

        void FileInfoTest::_ctors()
        {
            {
                const FileInfo f;
                TLRENDER_ASSERT(f.getPath().isEmpty());
            }
            {
                const Path path("tmp");
                {
                    FileIO::create(path.get(), Mode::Write);
                }
                const FileInfo f(path);
                TLRENDER_ASSERT(path == f.getPath());
                TLRENDER_ASSERT(Type::File == f.getType());
                TLRENDER_ASSERT(0 == f.getSize());
                TLRENDER_ASSERT(f.getPermissions() != 0);
                TLRENDER_ASSERT(f.getTime() != 0);
                rm(path.get());
            }
        }

        void FileInfoTest::_sequence()
        {
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(0, 2));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.0001.exr")));
                f.sequence(FileInfo(Path("test.0002.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.0000.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.1.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(1, 1));
            }
            {
                FileInfo f(Path("test.exr"));
                f.sequence(FileInfo(Path("test3.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test3.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(3, 3));
            }
            {
                FileInfo f(Path("test0999.exr"));
                f.sequence(FileInfo(Path("test1000.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(999, 1000));
            }
            {
                FileInfo f(Path("0001.exr"));
                f.sequence(FileInfo(Path("7800.exr")));
                TLRENDER_ASSERT(
                    f.getPath().getSequence() == math::IntRange(1, 7800));
            }
        }

        void FileInfoTest::_list()
        {
            {
                ListOptions options;
                options.sort = ListSort::Time;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != ListOptions());
            }

            std::string tmp = createTempDir();
            mkdir(file::Path(tmp, "dir").get());
            FileIO::create(file::Path(tmp, "file.txt").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.1.exr").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.2.exr").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.3.exr").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.1.tif").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.2.tif").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "render.3.tif").get(), Mode::Write);
            FileIO::create(
                file::Path(tmp, "render.0001.tif").get(), Mode::Write);
            FileIO::create(
                file::Path(tmp, "render.0002.tif").get(), Mode::Write);
            FileIO::create(
                file::Path(tmp, "render.0003.tif").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "movie.1.mov").get(), Mode::Write);
            FileIO::create(file::Path(tmp, "movie.2.mov").get(), Mode::Write);

            {
                std::vector<FileInfo> list;
                ListOptions options;
                options.sequence = true;
                options.sequenceExtensions = {".exr", ".tif"};
                file::list(tmp, list, options);
                TLRENDER_ASSERT(7 == list.size());
                for (size_t i = 0; i < list.size(); ++i)
                {
                    const auto& path = list[i].getPath();
                    if ("render." == path.getBaseName())
                    {
                        TLRENDER_ASSERT(path.isSequence());
                        TLRENDER_ASSERT(
                            path.getSequence() == math::IntRange(1, 3));
                    }
                }
                for (const auto i : {"movie.1.mov", "movie.2.mov"})
                {
                    const auto j = std::find_if(
                        list.begin(), list.end(),
                        [i](const FileInfo& value) {
                            return i ==
                                   value.getPath().get(-1, PathType::FileName);
                        });
                    TLRENDER_ASSERT(j != list.end());
                }

                options.sequence = false;
                file::list(tmp, list, options);
                TLRENDER_ASSERT(13 == list.size());
                for (size_t i = 0; i < list.size(); ++i)
                {
                    const auto& path = list[i].getPath();
                    if ("render." == path.getBaseName())
                    {
                        TLRENDER_ASSERT(!path.isSequence());
                    }
                }
            }

            std::vector<ListOptions> optionsList;
            for (auto sort : getListSortEnums())
            {
                ListOptions options;
                options.sort = sort;
                optionsList.push_back(options);
            }
            {
                ListOptions options;
                options.reverseSort = true;
                optionsList.push_back({options});
            }
            {
                ListOptions options;
                options.sortDirectoriesFirst = false;
                optionsList.push_back({options});
            }
            {
                ListOptions options;
                options.sequence = false;
                optionsList.push_back({options});
            }
            for (const auto& options : optionsList)
            {
                std::vector<FileInfo> list;
                file::list(tmp, list, options);
            }
        }
    } // namespace core_tests
} // namespace tl
