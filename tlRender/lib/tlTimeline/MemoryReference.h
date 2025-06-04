// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileIO.h>
#include <tlCore/Time.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

#include <memory>

namespace tl
{
    namespace timeline
    {
        //! Memory reference data.
        typedef std::vector<uint8_t> MemoryReferenceData;

        //! Read references from raw memory pointers.
        class RawMemoryReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "RawMemoryReference";
                static int constexpr version = 1;
            };

            RawMemoryReference(
                const std::string& target_url = std::string(),
                const uint8_t* memory = nullptr, size_t memory_size = 0,
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const uint8_t* memory() const noexcept;

            size_t memory_size() const noexcept;

            void set_memory(const uint8_t* memory, size_t memory_size);

        protected:
            virtual ~RawMemoryReference();

            std::string _target_url;
            const uint8_t* _memory = nullptr;
            size_t _memory_size = 0;
        };

        //! Read references from a shared memory pointer.
        class SharedMemoryReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SharedMemoryReference";
                static int constexpr version = 1;
            };

            SharedMemoryReference(
                const std::string& target_url = std::string(),
                const std::shared_ptr<MemoryReferenceData>& memory = nullptr,
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::shared_ptr<MemoryReferenceData>& memory() const noexcept;

            void set_memory(const std::shared_ptr<MemoryReferenceData>&);

        protected:
            virtual ~SharedMemoryReference();

            std::string _target_url;
            std::shared_ptr<MemoryReferenceData> _memory;
        };

        //! Read sequence references from raw memory pointers.
        class RawMemorySequenceReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "RawMemorySequenceReference";
                static int constexpr version = 1;
            };

            RawMemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& memory = {},
                const std::vector<size_t> memory_sizes = {},
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<const uint8_t*>& memory() const noexcept;

            const std::vector<size_t>& memory_sizes() const noexcept;

            void set_memory(
                const std::vector<const uint8_t*>& memory,
                const std::vector<size_t>& memory_sizes);

        protected:
            virtual ~RawMemorySequenceReference();

            std::string _target_url;
            std::vector<const uint8_t*> _memory;
            std::vector<size_t> _memory_sizes;
        };

        //! Read sequence references from shared memory pointers.
        class SharedMemorySequenceReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SharedMemorySequenceReference";
                static int constexpr version = 1;
            };

            SharedMemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<std::shared_ptr<MemoryReferenceData> >&
                    memory = {},
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<std::shared_ptr<MemoryReferenceData> >&
            memory() const noexcept;

            void set_memory(
                const std::vector<std::shared_ptr<MemoryReferenceData> >&);

        protected:
            virtual ~SharedMemorySequenceReference();

            std::string _target_url;
            std::vector<std::shared_ptr<MemoryReferenceData> > _memory;
        };

        //! Zip file memory reference for .otioz support.
        class ZipMemoryReference : public RawMemoryReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "ZipMemoryReference";
                static int constexpr version = 1;
            };

            ZipMemoryReference(
                const std::shared_ptr<file::FileIO>& file_io = nullptr,
                const std::string& target_url = std::string(),
                const uint8_t* memory = nullptr, size_t memory_size = 0,
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::shared_ptr<file::FileIO>& file_io() const noexcept;

            void set_file_io(const std::shared_ptr<file::FileIO>&);

        protected:
            virtual ~ZipMemoryReference();

            std::shared_ptr<file::FileIO> _file_io;
        };

        //! Zip file memory sequence reference for .otioz support.
        class ZipMemorySequenceReference : public RawMemorySequenceReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "ZipMemorySequenceReference";
                static int constexpr version = 1;
            };

            ZipMemorySequenceReference(
                const std::shared_ptr<file::FileIO>& file_io = nullptr,
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& memory = {},
                const std::vector<size_t> memory_sizes = {},
                const std::optional<otio::TimeRange>& available_range =
                    std::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::shared_ptr<file::FileIO>& file_io() const noexcept;

            void set_file_io(const std::shared_ptr<file::FileIO>&);

        protected:
            virtual ~ZipMemorySequenceReference();

            std::shared_ptr<file::FileIO> _file_io;
        };
    } // namespace timeline
} // namespace tl
