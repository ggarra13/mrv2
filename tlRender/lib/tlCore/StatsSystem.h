// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/MapObserver.h>
#include <tlCore/ValueObserver.h>

#include <string>

namespace tl
{
    namespace system
    {
        
        class Context;

        //! Statistics system.
        class StatsSystem : public ISystem
        {
            TLRENDER_NON_COPYABLE(StatsSystem);
        
        protected:
            StatsSystem(const std::shared_ptr<Context>&);

        public:
            virtual ~StatsSystem();

            //! Create a new system.
            static std::shared_ptr<StatsSystem> create(const std::shared_ptr<Context>&);

            //! Add a sampler function.
            void addSampler(const std::string&, const std::function<int64_t(void)>&);

            //! Get the sampler groups.
            const std::vector<std::string>& getGroups() const;

            //! Get whether the sampler exists.
            bool hasSampler(const std::string&) const;

            //! Get the maximum number of samples.
            size_t getSamplesMax() const;

            //! Observe the maximum number of samples.
            std::shared_ptr<observer::IValue<size_t> > observeSamplesMax() const;

            //! Set the maximum number of samples.
            void setSamplesMax(size_t);

            //! Get the samples.
            const std::map<std::string, std::vector<int64_t> >& getSamples() const;

            //! Observe the samples.
            std::shared_ptr<observer::IMap<std::string, std::vector<int64_t> > > observeSamples() const;

            //! Observe the samples increments.
            std::shared_ptr<observer::IMap<std::string, int64_t> > observeSamplesInc() const;

            void tick() override;

            TLRENDER_PRIVATE();
        };
    }
}
