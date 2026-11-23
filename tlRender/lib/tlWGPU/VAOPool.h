// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/Mesh.h>
#include <tlWGPU/WGPU.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace wgpu
    {

        // ---------------------------------------------------------------
        //  VAOAllocation
        //  Lightweight token returned by VAOPool::upload().
        //  Identifies which pool slot holds the data and where it starts.
        // ---------------------------------------------------------------
        struct VAOAllocation
        {
            std::size_t poolIndex = 0;   //< Index into VAOPool::_slots
            uint64_t offset = 0;         //< Byte offset within that slot's buffer
            std::size_t vertexCount = 0; //< Vertex count for the draw call
        };

        // ---------------------------------------------------------------
        //  VAOPool
        //
        //  Manages a list of identically-sized VAOs.  A WebGPU buffer is
        //  capped by WGPULimits::maxBufferSize (256 MiB by default, a few
        //  GiB at most even when raised), so unlike the Vulkan version a
        //  single giant buffer is not an option.  When the current slot runs
        //  out of space a new slot is allocated on demand.  Slots accumulate
        //  across frames and are reused; they are never freed while the pool
        //  is alive.
        //
        //  Usage pattern:
        //
        //    // Once per frame, before any uploads:
        //    pool->beginFrame();
        //
        //    // For each mesh:
        //    VAOAllocation alloc = pool->upload(vbo);
        //    pool->draw(pass, alloc);
        // ---------------------------------------------------------------
        class VAOPool
        {
        public:
            //! Create the pool.  @p slotSize is the size in bytes of each
            //! VAO slot's buffer.  It is clamped to the device's
            //! maxBufferSize.  @p device and @p queue are not owned.
            static std::shared_ptr<VAOPool> create(
                Fl_WebGPU_Context&,
                uint64_t slotSize = 128ull * 1024 * 1024);

            //! Call once per frame before any uploads, after the previous
            //! frame's commands were submitted.  Rewinds every slot and the
            //! current-slot cursor.
            void beginFrame();

            //! Upload vertex data.  Advances to a fresh slot if the current
            //! one has no room.  Throws only when a single upload is larger
            //! than one slot (irrecoverable: a draw can't span two buffers).
            VAOAllocation upload(
                const std::vector<uint8_t>& data, std::size_t vertexCount);

            //! Convenience overload - pulls data and vertex count from @p vbo.
            VAOAllocation upload(const std::shared_ptr<VBO>& vbo);

            //! Issue the draw command for a previously uploaded allocation.
            void draw(WGPURenderPassEncoder pass, const VAOAllocation& alloc);

            //! Return the number of VAO slots currently allocated.
            std::size_t slotCount() const;

            //! Return the size in bytes of each slot.
            uint64_t slotSize() const;

        private:
            VAOPool(Fl_WebGPU_Context&, uint64_t slotSize);

            //! Ensure slot @p index exists, creating it if necessary.
            void _ensureSlot(std::size_t index);

            WGPUDevice _device = nullptr;
            WGPUQueue _queue = nullptr;
            uint64_t _slotSize = 0;
            std::size_t _currentSlot = 0;
            Fl_WebGPU_Context& _ctx;

            std::vector<std::shared_ptr<VAO> > _slots;
        };

    } // namespace webgpu
} // namespace tl
