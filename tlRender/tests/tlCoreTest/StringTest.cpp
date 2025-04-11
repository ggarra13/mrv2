// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/StringTest.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>
#include <sstream>

using namespace tl::string;

namespace tl
{
    namespace core_tests
    {
        StringTest::StringTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::StringTest", context)
        {
        }

        std::shared_ptr<StringTest>
        StringTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<StringTest>(new StringTest(context));
        }

        void StringTest::run()
        {
            _split();
            _format();
            _compare();
            _convert();
            _escape();
        }

        void StringTest::_split()
        {
            {
                const auto pieces = split("", '/');
                TLRENDER_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("/", '/');
                TLRENDER_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("a", '/');
                TLRENDER_ASSERT(1 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("/a", '/');
                TLRENDER_ASSERT(1 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("a/", '/');
                TLRENDER_ASSERT(1 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("a/b/c//", '/');
                TLRENDER_ASSERT(3 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
                TLRENDER_ASSERT("b" == pieces[1]);
                TLRENDER_ASSERT("c" == pieces[2]);
            }
            {
                const auto pieces =
                    split("a/b/c//", '/', SplitOptions::KeepEmpty);
                TLRENDER_ASSERT(4 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
                TLRENDER_ASSERT("b" == pieces[1]);
                TLRENDER_ASSERT("c" == pieces[2]);
                TLRENDER_ASSERT(pieces[3].empty());
            }
            {
                const auto pieces = split("", {'/', '|'});
                TLRENDER_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("|", {'/', '|'});
                TLRENDER_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("a", {'/', '|'});
                TLRENDER_ASSERT(1 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("a/b|c||", {'/', '|'});
                TLRENDER_ASSERT(3 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
                TLRENDER_ASSERT("b" == pieces[1]);
                TLRENDER_ASSERT("c" == pieces[2]);
            }
            {
                const auto pieces =
                    split("a/b|c||", {'/', '|'}, SplitOptions::KeepEmpty);
                TLRENDER_ASSERT(4 == pieces.size());
                TLRENDER_ASSERT("a" == pieces[0]);
                TLRENDER_ASSERT("b" == pieces[1]);
                TLRENDER_ASSERT("c" == pieces[2]);
                TLRENDER_ASSERT(pieces[3].empty());
            }
            {
                TLRENDER_ASSERT("a/b/c" == join({"a", "b", "c"}, "/"));
            }
        }

        void StringTest::_format()
        {
            {
                TLRENDER_ASSERT("ABC" == toUpper("abc"));
                TLRENDER_ASSERT("abc" == toLower("ABC"));
            }
            {
                std::string s = "abc";
                removeTrailingNewlines(s);
                TLRENDER_ASSERT("abc" == s);
                s = "abc\n";
                removeTrailingNewlines(s);
                TLRENDER_ASSERT("abc" == s);
                s = "abc\r";
                removeTrailingNewlines(s);
                TLRENDER_ASSERT("abc" == s);
                s = "abc\n\r";
                removeTrailingNewlines(s);
                TLRENDER_ASSERT("abc" == s);
            }
            {
                const std::string s = "abc\n";
                TLRENDER_ASSERT(removeTrailingNewlines(s) == "abc");
            }
            {
                TLRENDER_ASSERT("abc" == elide("abc", 3));
                TLRENDER_ASSERT("abc" == elide("abc", 4));
                TLRENDER_ASSERT("ab..." == elide("abc", 2));
                TLRENDER_ASSERT("..." == elide("abc", 0));
            }
        }

        void StringTest::_compare()
        {
            {
                TLRENDER_ASSERT(!compare("abc", "ABC"));
                TLRENDER_ASSERT(
                    compare("abc", "ABC", Compare::CaseInsensitive));
            }
            {
                TLRENDER_ASSERT(!contains("abc", "C"));
                TLRENDER_ASSERT(contains("abc", "C", Compare::CaseInsensitive));
            }
        }

        void StringTest::_convert()
        {
            {
                _print(string::Format("{0}/{1}")
                           .arg(getLabel(true))
                           .arg(getLabel(false)));
            }
            {
                int value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLRENDER_ASSERT(1234 == value);
            }
            {
                int value = 0;
                char buf[] = "+1234";
                fromString(buf, 5, value);
                TLRENDER_ASSERT(1234 == value);
            }
            {
                int value = 0;
                char buf[] = "-1234";
                fromString(buf, 5, value);
                TLRENDER_ASSERT(-1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLRENDER_ASSERT(1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "+1234";
                fromString(buf, 5, value);
                TLRENDER_ASSERT(1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "-1234";
                fromString(buf, 5, value);
                TLRENDER_ASSERT(-1234 == value);
            }
            {
                size_t value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLRENDER_ASSERT(1234 == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLRENDER_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "+1234.0";
                fromString(buf, 7, value);
                TLRENDER_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "-1234.0";
                fromString(buf, 7, value);
                TLRENDER_ASSERT(-1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234e0";
                fromString(buf, 6, value);
                TLRENDER_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234e1";
                fromString(buf, 6, value);
                TLRENDER_ASSERT(12340.F == value);
            }
            {
                TLRENDER_ASSERT("abc" == fromWide(toWide("abc")));
            }
        }

        void StringTest::_escape()
        {
            {
                TLRENDER_ASSERT("\\\\" == escape("\\"));
                TLRENDER_ASSERT("\\" == unescape("\\\\"));
            }
        }
    } // namespace core_tests
} // namespace tl
