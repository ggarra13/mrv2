// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>

#include <tlCore/Time.h>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            class ITestPattern
                : public std::enable_shared_from_this<ITestPattern>
            {
                TLRENDER_NON_COPYABLE(ITestPattern);

            protected:
                void _init(
                    const std::string&, const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

                ITestPattern();

            public:
                virtual ~ITestPattern() = 0;

                const std::string& getName() const;

                virtual void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) = 0;

            protected:
                std::weak_ptr<system::Context> _context;
                std::string _name;
                math::Size2i _size;
            };

            class CountTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

            public:
                virtual ~CountTestPattern();

                static std::string getClassName();

                static std::shared_ptr<CountTestPattern> create(
                    const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;

            private:
                image::FontInfo _secondsFontInfo;
                image::FontMetrics _secondsFontMetrics;
                image::FontInfo _framesFontInfo;
                image::FontMetrics _framesFontMetrics;
            };

            class SwatchesTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

            public:
                virtual ~SwatchesTestPattern();

                static std::string getClassName();

                static std::shared_ptr<SwatchesTestPattern> create(
                    const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;

            private:
                std::shared_ptr<image::Image> _gradient;
            };

            class GridTestPattern : public ITestPattern
            {
            public:
                virtual ~GridTestPattern();

                static std::string getClassName();

                static std::shared_ptr<GridTestPattern> create(
                    const math::Size2i&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;
            };

            class TestPatternFactory
            {
            public:
                static std::shared_ptr<ITestPattern> create(
                    const std::string& name, const math::Size2i&,
                    const std::shared_ptr<system::Context>&);
            };
        } // namespace test_patterns
    } // namespace examples
} // namespace tl
