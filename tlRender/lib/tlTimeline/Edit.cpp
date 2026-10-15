// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Edit.h>

#include <tlTimeline/MemoryReference.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <algorithm>

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

                virtual void copy(OTIO_NS::MediaReference*) = 0;
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

                void copy(OTIO_NS::MediaReference* value) override
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

                void copy(OTIO_NS::MediaReference* value) override
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

                void copy(OTIO_NS::MediaReference* value) override
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

                void copy(OTIO_NS::MediaReference* value) override
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

                void copy(OTIO_NS::MediaReference* value) override
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

                void copy(OTIO_NS::MediaReference* value) override
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
            EditMode, "None", "Fill", "Insert",
            "Move", "Overwrite", "Ripple", "Roll", "Select",
            "Slice", "Slip", "Slide", "Trim");
        TLRENDER_ENUM_SERIALIZE_IMPL(EditMode);

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>
        copy(const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline)
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
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> out(
                dynamic_cast<OTIO_NS::Timeline*>(
                    OTIO_NS::Timeline::from_json_string(s)));

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

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> move(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
            const std::vector<MoveData>& moves)
        {
            auto out = copy(timeline);
            const auto& tracks = out->tracks()->children();

            auto getTrack = [&tracks](int index) -> OTIO_NS::Track*
            {
                if (index < 0 || index >= static_cast<int>(tracks.size()))
                    return nullptr;
                return OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(
                    tracks[index]);
            };

            // All the moves (the clicked item plus its associated items) are
            // expressed as indexes into the ORIGINAL timeline.  Applying them
            // one by one would invalidate the indexes of the later moves
            // (for example, when several items live on the same track), so we
            // first resolve every move to the items themselves, and then
            // apply them by pointer.
            struct ResolvedMove
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> item;
                OTIO_NS::Track* fromTrack = nullptr;
                OTIO_NS::Track* toTrack = nullptr;
                int toIndex = 0;
                //! Item the moved item is inserted in front of.  A nullptr
                //! means append to the end of the destination track.
                OTIO_NS::Composable* anchor = nullptr;
            };
            std::vector<ResolvedMove> resolved;
            std::vector<OTIO_NS::Composable*> moving;

            // Pass 1: resolve the items being moved (skipping duplicates, as
            // an item can be both selected and associated with another one).
            for (const auto& move : moves)
            {
                auto fromTrack = getTrack(move.fromTrack);
                auto toTrack = getTrack(move.toTrack);
                if (!fromTrack || !toTrack)
                    continue;
                if (move.fromOtioIndex < 0 ||
                    move.fromOtioIndex >=
                        static_cast<int>(fromTrack->children().size()))
                    continue;

                auto child = fromTrack->children()[move.fromOtioIndex];
                if (!child.value ||
                    std::find(moving.begin(), moving.end(), child.value) !=
                        moving.end())
                    continue;

                ResolvedMove r;
                r.item = child;
                r.fromTrack = fromTrack;
                r.toTrack = toTrack;
                r.toIndex = move.toOtioIndex;
                resolved.push_back(r);
                moving.push_back(child.value);
            }

            // Pass 2: resolve the anchor each item is inserted in front of:
            // the first child at or after the destination index that is not
            // itself being moved.
            for (auto& r : resolved)
            {
                const auto& children = r.toTrack->children();
                for (int i = std::max(r.toIndex, 0);
                     i < static_cast<int>(children.size()); ++i)
                {
                    if (std::find(
                            moving.begin(), moving.end(), children[i].value) ==
                        moving.end())
                    {
                        r.anchor = children[i].value;
                        break;
                    }
                }
            }

            // Pass 3: apply the moves.  Items moved to the same anchor keep
            // the order in which they were given.
            for (auto& r : resolved)
            {
                const int fromIndex = r.fromTrack->index_of_child(r.item.value);
                if (fromIndex < 0)
                    continue;
                r.fromTrack->remove_child(fromIndex);

                int toIndex = static_cast<int>(r.toTrack->children().size());
                if (r.anchor)
                {
                    const int anchorIndex = r.toTrack->index_of_child(r.anchor);
                    if (anchorIndex >= 0)
                        toIndex = anchorIndex;
                }
                r.toTrack->insert_child(toIndex, r.item);
            }
            return out;
        }
    } // namespace timeline
} // namespace tl
