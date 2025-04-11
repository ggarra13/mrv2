// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtQuick/Init.h>

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlQt/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace qtquick
    {
        namespace
        {
            std::shared_ptr<system::Context> _context;
        }

        void init(
            qt::DefaultSurfaceFormat defaultSurfaceFormat,
            const std::shared_ptr<system::Context>& context)
        {
            qt::init(defaultSurfaceFormat, context);
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
        }

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::qtquick::System", context);

            _context = context;

            qmlRegisterType<GLFramebufferObject>(
                "tlQtQuick", 1, 0, "GLFramebufferObject");
        }

        System::System() {}

        System::~System()
        {
            _context.reset();
        }

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(context);
            return out;
        }

        const std::shared_ptr<system::Context>& getContext()
        {
            return _context;
        }
    } // namespace qtquick
} // namespace tl
