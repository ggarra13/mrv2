// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ContextTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/ISystem.h>

namespace tl
{
    namespace core_tests
    {
        ContextTest::ContextTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ContextTest", context)
        {
        }

        std::shared_ptr<ContextTest>
        ContextTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ContextTest>(new ContextTest(context));
        }

        namespace
        {
            class TestSystem : public system::ISystem
            {
                TLRENDER_NON_COPYABLE(TestSystem);

            protected:
                void _init(const std::shared_ptr<system::Context>& context)
                {
                    ISystem::_init("TestSystem", context);
                    _log("Hello world!");
                    _log("Hello world!", log::Type::Warning);
                    _log("Hello world!", log::Type::Error);
                }

                TestSystem() {}

            public:
                virtual ~TestSystem()
                {
                    std::stringstream ss;
                    ss << "Ticks: " << _ticks;
                    _log(ss.str());
                }

                static std::shared_ptr<TestSystem>
                create(const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<TestSystem>(new TestSystem);
                    out->_init(context);
                    return out;
                }

                void tick() override
                {
                    ISystem::tick();
                    ++_ticks;
                }

                std::chrono::milliseconds getTickTime() const override
                {
                    return std::chrono::milliseconds(1);
                }

            private:
                size_t _ticks = 0;
            };
        } // namespace

        void ContextTest::run()
        {
            {
                auto testSystem = TestSystem::create(_context);
                TLRENDER_ASSERT(testSystem->getContext().lock());
                {
                    std::stringstream ss;
                    ss << "Name: " << testSystem->getName();
                    _print(ss.str());
                }
                TLRENDER_ASSERT(!_context->getSystem<TestSystem>());
                _context->addSystem(testSystem);
                TLRENDER_ASSERT(
                    testSystem == _context->getSystem<TestSystem>());
                for (size_t i = 0; i < 10; ++i)
                {
                    _context->tick();
                }
            }
        }
    } // namespace core_tests
} // namespace tl
