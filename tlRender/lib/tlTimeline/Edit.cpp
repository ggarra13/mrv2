// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Edit.h>

#include <tlTimeline/MemoryReference.h>

#include <tlCore/Error.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            class IMemoryData
            {
            protected:
                IMemoryData();

            public:
                virtual ~IMemoryData() = 0;

                virtual void copy(otio::MediaReference*) = 0;
            };

            IMemoryData::IMemoryData() {}

            IMemoryData::~IMemoryData() {}

            class RawMemoryData : public IMemoryData
            {
            protected:
                void _init(const RawMemoryReference* value)
                {
                    _target_url = value->target_url();
                    _memory = value->memory();
                    _memory_size = value->memory_size();
                }

                RawMemoryData() {}

            public:
                static std::shared_ptr<RawMemoryData>
                create(const RawMemoryReference* value)
                {
                    std::shared_ptr<RawMemoryData> out(new RawMemoryData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    if (auto ref = dynamic_cast<RawMemoryReference*>(value))
                    {
                        ref->set_target_url(_target_url);
                        ref->set_memory(_memory, _memory_size);
                    }
                }

            private:
                std::string _target_url;
                const uint8_t* _memory = nullptr;
                size_t _memory_size = 0;
            };

            class SharedMemoryData : public IMemoryData
            {
            protected:
                void _init(const SharedMemoryReference* value)
                {
                    _target_url = value->target_url();
                    _memory = value->memory();
                }

                SharedMemoryData() {}

            public:
                static std::shared_ptr<SharedMemoryData>
                create(const SharedMemoryReference* value)
                {
                    std::shared_ptr<SharedMemoryData> out(new SharedMemoryData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    if (auto ref = dynamic_cast<SharedMemoryReference*>(value))
                    {
                        ref->set_target_url(_target_url);
                        ref->set_memory(_memory);
                    }
                }

            private:
                std::string _target_url;
                std::shared_ptr<MemoryReferenceData> _memory;
            };

            class RawMemorySequenceData : public IMemoryData
            {
            protected:
                void _init(const RawMemorySequenceReference* value)
                {
                    _target_url = value->target_url();
                    _memory = value->memory();
                    _memory_sizes = value->memory_sizes();
                }

                RawMemorySequenceData() {}

            public:
                static std::shared_ptr<RawMemorySequenceData>
                create(const RawMemorySequenceReference* value)
                {
                    std::shared_ptr<RawMemorySequenceData> out(
                        new RawMemorySequenceData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    if (auto ref =
                            dynamic_cast<RawMemorySequenceReference*>(value))
                    {
                        ref->set_target_url(_target_url);
                        ref->set_memory(_memory, _memory_sizes);
                    }
                }

            private:
                std::string _target_url;
                std::vector<const uint8_t*> _memory;
                std::vector<size_t> _memory_sizes;
            };

            class SharedMemorySequenceData : public IMemoryData
            {
            protected:
                void _init(const SharedMemorySequenceReference* value)
                {
                    _target_url = value->target_url();
                    _memory = value->memory();
                }

                SharedMemorySequenceData() {}

            public:
                static std::shared_ptr<SharedMemorySequenceData>
                create(const SharedMemorySequenceReference* value)
                {
                    std::shared_ptr<SharedMemorySequenceData> out(
                        new SharedMemorySequenceData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    if (auto ref =
                            dynamic_cast<SharedMemorySequenceReference*>(value))
                    {
                        ref->set_target_url(_target_url);
                    }
                }

            private:
                std::string _target_url;
                std::vector<std::shared_ptr<MemoryReferenceData> > _memory;
            };

            class ZipMemoryData : public RawMemoryData
            {
            protected:
                void _init(const ZipMemoryReference* value)
                {
                    RawMemoryData::_init(value);
                    _file_io = value->file_io();
                }

                ZipMemoryData() {}

            public:
                static std::shared_ptr<ZipMemoryData>
                create(const ZipMemoryReference* value)
                {
                    std::shared_ptr<ZipMemoryData> out(new ZipMemoryData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    RawMemoryData::copy(value);
                    if (auto ref = dynamic_cast<ZipMemoryReference*>(value))
                    {
                        ref->set_file_io(_file_io);
                    }
                }

            private:
                std::shared_ptr<file::FileIO> _file_io;
            };

            class ZipMemorySequenceData : public RawMemorySequenceData
            {
            protected:
                void _init(const ZipMemorySequenceReference* value)
                {
                    RawMemorySequenceData::_init(value);
                    _file_io = value->file_io();
                }

                ZipMemorySequenceData() {}

            public:
                static std::shared_ptr<ZipMemorySequenceData>
                create(const ZipMemorySequenceReference* value)
                {
                    std::shared_ptr<ZipMemorySequenceData> out(
                        new ZipMemorySequenceData);
                    out->_init(value);
                    return out;
                }

                void copy(otio::MediaReference* value) override
                {
                    RawMemorySequenceData::copy(value);
                    if (auto ref = dynamic_cast<ZipMemoryReference*>(value))
                    {
                        ref->set_file_io(_file_io);
                    }
                }

            private:
                std::shared_ptr<file::FileIO> _file_io;
            };
        } // namespace

        TLRENDER_ENUM_IMPL(
            EditMode, "None", "Fill", "Move", "Ripple", "Roll", "Select",
            "Slice", "Slip", "Slide", "Trim");
        TLRENDER_ENUM_SERIALIZE_IMPL(EditMode);
        
        otio::SerializableObject::Retainer<otio::Timeline>
        copy(const otio::SerializableObject::Retainer<otio::Timeline>& timeline)
        {
            //! \todo Since we are copying the timeline by serializing it to
            //! JSON, we need to keep track of in-memory media references and
            //! copy them to the new timeline. Would it be better to make a
            //! deep copy of the timeline to avoid this?
            std::vector<std::shared_ptr<IMemoryData> > memoryData;
            for (const auto& clip : timeline->find_clips())
            {
                if (auto ref = dynamic_cast<ZipMemoryReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(ZipMemoryData::create(ref));
                }
                else if (
                    auto ref = dynamic_cast<RawMemoryReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(RawMemoryData::create(ref));
                }
                else if (
                    auto ref = dynamic_cast<SharedMemoryReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(SharedMemoryData::create(ref));
                }
                else if (
                    auto ref = dynamic_cast<ZipMemorySequenceReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(ZipMemorySequenceData::create(ref));
                }
                else if (
                    auto ref = dynamic_cast<RawMemorySequenceReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(RawMemorySequenceData::create(ref));
                }
                else if (
                    auto ref = dynamic_cast<SharedMemorySequenceReference*>(
                        clip->media_reference()))
                {
                    ref->metadata()["tlRender"] =
                        static_cast<int64_t>(memoryData.size());
                    memoryData.push_back(SharedMemorySequenceData::create(ref));
                }
            }

            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(
                    otio::Timeline::from_json_string(s)));

            for (const auto& clip : out->find_clips())
            {
                if (dynamic_cast<RawMemoryReference*>(
                        clip->media_reference()) ||
                    dynamic_cast<SharedMemoryReference*>(
                        clip->media_reference()) ||
                    dynamic_cast<RawMemorySequenceReference*>(
                        clip->media_reference()) ||
                    dynamic_cast<SharedMemorySequenceReference*>(
                        clip->media_reference()) ||
                    dynamic_cast<ZipMemoryReference*>(
                        clip->media_reference()) ||
                    dynamic_cast<ZipMemorySequenceReference*>(
                        clip->media_reference()))
                {
                    auto ref = clip->media_reference();
                    const auto i = ref->metadata().find("tlRender");
                    if (i != ref->metadata().end())
                    {
                        const int64_t index = std::any_cast<int64_t>(i->second);
                        if (index >= 0 && index < memoryData.size())
                        {
                            memoryData[index]->copy(ref);
                        }
                        ref->metadata().erase(i);
                    }
                }
            }

            return out;
        }

        otio::SerializableObject::Retainer<otio::Timeline> move(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::vector<MoveData>& moves)
        {
            auto out = copy(timeline);
            for (const auto& move : moves)
            {
                if (move.fromTrack >= 0 &&
                    move.fromTrack < out->tracks()->children().size() &&
                    move.toTrack >= 0 &&
                    move.toTrack < out->tracks()->children().size())
                {
                    int toIndex = move.toIndex;
                    if (move.fromTrack == move.toTrack &&
                        move.fromIndex < toIndex)
                    {
                        --toIndex;
                    }
                    if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                            out->tracks()->children()[move.fromTrack]))
                    {
                        auto child = track->children()[move.fromIndex];
                        track->remove_child(move.fromIndex);

                        if (auto track =
                                otio::dynamic_retainer_cast<otio::Track>(
                                    out->tracks()->children()[move.toTrack]))
                        {
                            track->insert_child(toIndex, child);
                        }
                    }
                }
            }
            return out;
        }
    } // namespace timeline
} // namespace tl
