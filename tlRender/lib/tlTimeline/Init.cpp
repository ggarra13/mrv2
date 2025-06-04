// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Init.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/Init.h>

#include <tlCore/Context.h>
#include <tlCore/PathMapping.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/typeRegistry.h>

namespace tl
{
    namespace timeline
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            io::init(context);
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
            path_mapping::init();
        }

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::timeline::System", context);
            const std::vector<std::pair<std::string, bool> > registerTypes{
                {"RawMemoryReference",
                 otio::TypeRegistry::instance()
                     .register_type<tl::timeline::RawMemoryReference>()},
                {"SharedMemoryReference",
                 otio::TypeRegistry::instance()
                     .register_type<tl::timeline::SharedMemoryReference>()},
                {"RawMemorySequenceReference",
                 otio::TypeRegistry::instance()
                     .register_type<
                         tl::timeline::RawMemorySequenceReference>()},
                {"SharedMemorySequenceReference",
                 otio::TypeRegistry::instance()
                     .register_type<
                         tl::timeline::SharedMemorySequenceReference>()},
                {"ZipMemoryReference",
                 otio::TypeRegistry::instance()
                     .register_type<tl::timeline::ZipMemoryReference>()},
                {"ZipMemorySequenceReference",
                 otio::TypeRegistry::instance()
                     .register_type<
                         tl::timeline::ZipMemorySequenceReference>()}};
            for (const auto& t : registerTypes)
            {
                _log(string::Format("register type {0}: {1}")
                         .arg(t.first)
                         .arg(t.second));
            }
        }

        System::System() {}

        System::~System() {}

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(context);
            return out;
        }
    } // namespace timeline
} // namespace tl
