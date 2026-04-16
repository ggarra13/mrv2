// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Mesh.h>
#include <tlVk/Vk.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace vlk
    {

        // ---------------------------------------------------------------
        //  VAOAllocation
        //  Lightweight token returned by VAOPool::upload().
        //  Identifies which pool slot holds the data and where it starts.
        // ---------------------------------------------------------------
        struct VAOAllocation
        {
            std::size_t  poolIndex   = 0; //< Index into VAOPool::_slots
            VkDeviceSize offset      = 0; //< Absolute offset within that VAO's buffer
            std::size_t  vertexCount = 0; //< Vertex count for the draw call
        };

        // ---------------------------------------------------------------
        //  VAOPool
        //
        //  Manages a list of identically-sized VAOs.  When the current
        //  slot runs out of space a new 3.5 GB VAO is allocated on demand.
        //  Slots accumulate across frames and are reused; they are never
        //  freed while the pool is alive.
        //
        //  Usage pattern (mirrors single-VAO usage):
        //
        //    // Once per frame, before any draws:
        //    pool->bind(frameIndex);
        //
        //    // For each mesh:
        //    VAOAllocation alloc = pool->upload(vbo->getData(), vbo->getSize());
        //    pool->draw(cmd, alloc);
        // ---------------------------------------------------------------
        class VAOPool
        {
        public:
            //! Create the pool. @p slotSize is the total VkBuffer size for
            //! each individual VAO slot (e.g. 3.5 * memory::gigabyte).
            static std::shared_ptr<VAOPool> create(Fl_Vk_Context& ctx,
                                                   VkDeviceSize   slotSize);

            //! Call once per frame before any uploads.
            //! Resets all existing slots to the new frame and rewinds the
            //! current-slot cursor to 0 so slots are filled from the first
            //! one again.
            void bind(uint32_t frameIndex);

            //! Upload vertex data.  Advances to a fresh slot if the current
            //! one has no room.  Throws only when a single upload is larger
            //! than one slot's per-frame region (irrecoverable).
            VAOAllocation upload(const std::vector<uint8_t>& data,
                                 std::size_t                 vertexCount);

            //! Convenience overload – pulls data and vertex count from @p vbo.
            VAOAllocation upload(const std::shared_ptr<VBO>& vbo);

            //! Issue the draw command for a previously uploaded allocation.
            void draw(VkCommandBuffer& cmd, const VAOAllocation& alloc);

            //! Return the number of VAO slots currently allocated.
            std::size_t slotCount() const;

        private:
            VAOPool(Fl_Vk_Context& ctx, VkDeviceSize slotSize);

            //! Ensure slot @p index exists, creating and initialising it if
            //! necessary.
            void _ensureSlot(std::size_t index);

            Fl_Vk_Context& _ctx;
            VkDeviceSize   _slotSize    = 0;
            uint32_t       _frameIndex  = 0;
            std::size_t    _currentSlot = 0;

            std::vector<std::shared_ptr<VAO>> _slots;
        };

    } // namespace vlk
} // namespace tl
