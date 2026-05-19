// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#include <tlCore/StatsSystem.h>

#include <tlCore/Audio.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Image.h>
#include <tlCore/String.h>

namespace tl
{

    namespace system
    {
        
        struct StatsSystem::Private
        {
            std::vector<std::pair<std::string, std::function<int64_t(void)> > > samplers;
            std::vector<std::string> groups;
            std::map<std::string, std::vector<std::string> > names;
            std::shared_ptr<observer::Value<size_t> > samplesMax;
            std::shared_ptr<observer::Map<std::string, std::vector<int64_t> > > samples;
            std::shared_ptr<observer::Map<std::string, int64_t> > samplesInc;
        };

        StatsSystem::StatsSystem(const std::shared_ptr<Context>& context) :
            ISystem(),
            _p(new Private)
        {
            TLRENDER_P();
            p.samplesMax = observer::Value<size_t>::create(100);
            p.samples = observer::Map<std::string, std::vector<int64_t> >::create();
            p.samplesInc = observer::Map<std::string, int64_t>::create();

            addSampler(
                "tlRender Memory/Images: {0}MB",
                [] { return image::Image::getTotalByteCount() / memory::megabyte; });
            addSampler(
                "tlRender Memory/Audio: {0}MB",
                [] { return audio::Audio::getTotalByteCount() / memory::megabyte; });

            addSampler(
                "tlRender Objects/Images: {0}",
                [] { return image::Image::getObjectCount(); });

            addSampler(
                "tlRender Objects/Audio: {0}",
                [] { return audio::Audio::getObjectCount(); });
        }

        StatsSystem::~StatsSystem()
        {}

        std::shared_ptr<StatsSystem> StatsSystem::create(
            const std::shared_ptr<Context>& context)
        {
            return std::shared_ptr<StatsSystem>(new StatsSystem(context));
        }

        void StatsSystem::addSampler(const std::string& id, const std::function<int64_t(void)>& sampler)
        {
            TLRENDER_P();
            auto i = id.find_first_of('/');
            if (!hasSampler(id) && i != std::string::npos)
            {
                p.samplers.push_back(std::make_pair(id, sampler));
                const std::string group = id.substr(0, i);
                const std::string name = id.substr(i + 1);
                auto i = std::find(p.groups.begin(), p.groups.end(), group);
                if (i == p.groups.end())
                {
                    p.groups.push_back(group);
                }
                p.names[group].push_back(name);
            }
        }

        const std::vector<std::string>& StatsSystem::getGroups() const
        {
            return _p->groups;
        }

        std::vector<std::string> StatsSystem::getNames(const std::string& group) const
        {
            TLRENDER_P();
            const auto i = p.names.find(group);
            return i != p.names.end() ? i->second : std::vector<std::string>();
        }

        bool StatsSystem::hasSampler(const std::string& id) const
        {
            TLRENDER_P();
            const auto i = std::find_if(
                p.samplers.begin(),
                p.samplers.end(),
                [id](const std::pair<std::string, std::function<int64_t(void)> >& value)
                    {
                        return id == value.first;
                    });
            return i != p.samplers.end();
        }

        size_t StatsSystem::getSamplesMax() const
        {
            return _p->samplesMax->get();
        }

        std::shared_ptr<observer::IValue<size_t> > StatsSystem::observeSamplesMax() const
        {
            return _p->samplesMax;
        }

        void StatsSystem::setSamplesMax(size_t value)
        {
            TLRENDER_P();
            if (p.samplesMax->setIfChanged(value))
            {
                auto samples = p.samples->get();
                for (auto i : samples)
                {
                    if (i.second.size() > value)
                    {
                        std::list<int64_t> tmp(i.second.begin(), i.second.end());
                        while (tmp.size() > value)
                        {
                            tmp.pop_front();
                        }
                        i.second = std::vector<int64_t>(tmp.begin(), tmp.end());
                    }
                }
                p.samples->setIfChanged(samples);
            }
        }

        const std::map<std::string, std::vector<int64_t> >& StatsSystem::getSamples() const
        {
            return _p->samples->get();
        }

        std::shared_ptr<observer::IMap<std::string, std::vector<int64_t> > > StatsSystem::observeSamples() const
        {
            return _p->samples;
        }

        std::shared_ptr<observer::IMap<std::string, int64_t> > StatsSystem::observeSamplesInc() const
        {
            return _p->samplesInc;
        }

        void StatsSystem::tick()
        {
            TLRENDER_P();
            std::map<std::string, int64_t> samplesInc;
            for (const auto& i : p.samplers)
            {
                samplesInc[i.first] = 0;
            }
            for (const auto& i : p.samplers)
            {
                samplesInc[i.first] += i.second();
            }
            p.samplesInc->setAlways(samplesInc);

            std::map<std::string, std::vector<int64_t> > samples = p.samples->get();
            for (auto i : samplesInc)
            {
                samples[i.first].push_back(i.second);
            }
            const size_t max = p.samplesMax->get();
            for (auto i : samples)
            {
                if (i.second.size() > max)
                {
                    std::list<int64_t> tmp(i.second.begin(), i.second.end());
                    while (tmp.size() > max)
                    {
                        tmp.pop_front();
                    }
                    i.second = std::vector<int64_t>(tmp.begin(), tmp.end());
                }
            }
            p.samples->setAlways(samples);
        }

        void StatsSystem::_log()
        {
            TLRENDER_P();
            const auto samples = p.samples->get();
            std::vector<std::string> lines;
            lines.push_back(std::string());
            for (const auto& group : p.groups)
            {
                lines.push_back(string::Format("    {0}:").arg(group));
                for (const auto& name : p.names[group])
                {
                    const auto i = samples.find(group + "/" + name);
                    if (i != samples.end())
                    {
                        std::string v = string::Format(name).arg(!i->second.empty() ?
                                                                 i->second.back() : 0);
                        lines.push_back(string::Format("    * {0}").arg(v));
                    }
                }
            }
            ISystem::_log(string::join(lines, '\n'));
        }
    }
}
