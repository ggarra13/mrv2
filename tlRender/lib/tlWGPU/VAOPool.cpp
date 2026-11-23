// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlWGPU/VAOPool.h>

#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <stdexcept>

namespace tl
{
    namespace wgpu
    {

        // ------------------------------------------------------------------
        //  VAOPool
        // ------------------------------------------------------------------

        VAOPool::VAOPool(Fl_WebGPU_Context& ctx, uint64_t slotSize) :
            _ctx(ctx),
            _device(ctx.device),
            _queue(ctx.queue()),
            _slotSize(std::min(slotSize, VAO::getMaxMemorySize(_device)))
        {
            // Pre-allocate the first slot so the pool is ready to use
            // immediately after construction.
            _ensureSlot(0);
        }

        std::shared_ptr<VAOPool> VAOPool::create(
            Fl_WebGPU_Context& ctx, uint64_t slotSize)
        {
            // Can't use make_shared - constructor is private.
            return std::shared_ptr<VAOPool>(new VAOPool(ctx, slotSize));
        }

        void VAOPool::beginFrame()
        {
            _currentSlot = 0;

            // Rewind every existing slot; no per-frame partitioning or
            // frame index is needed with queue.writeBuffer().
            for (auto& slot : _slots)
                slot->beginFrame();
        }

        VAOAllocation VAOPool::upload(
            const std::vector<uint8_t>& data, std::size_t vertexCount)
        {
            // Fast path: current slot has room.
            if (!_slots[_currentSlot]->canFit(data.size()))
            {
                ++_currentSlot;
                _ensureSlot(_currentSlot);

                // If it still can't fit, the upload is larger than one slot,
                // which is an irrecoverable configuration error (a single
                // draw call can't span two buffers).
                if (!_slots[_currentSlot]->canFit(data.size()))
                {
                    std::string msg =
                        string::Format("VAOPool: single mesh upload "
                                       "of {0} bytes exceeds one slot's "
                                       "size of {1} MB.  "
                                       "Increase slotSize (and the device's "
                                       "maxBufferSize limit).")
                            .arg(data.size())
                            .arg(_slotSize / memory::megabyte);
                    throw std::runtime_error(msg);
                }
            }

            VAOAllocation alloc;
            alloc.poolIndex = _currentSlot;
            alloc.offset =
                static_cast<uint64_t>(_slots[_currentSlot]->upload(data));
            alloc.vertexCount = vertexCount;
            return alloc;
        }

        VAOAllocation VAOPool::upload(const std::shared_ptr<VBO>& vbo)
        {
            return upload(vbo->getData(), vbo->getSize());
        }

        void VAOPool::draw(WGPURenderPassEncoder pass, const VAOAllocation& alloc)
        {
            _slots[alloc.poolIndex]->draw(pass, alloc.vertexCount, alloc.offset);
        }

        std::size_t VAOPool::slotCount() const
        {
            return _slots.size();
        }

        uint64_t VAOPool::slotSize() const
        {
            return _slotSize;
        }

        void VAOPool::_ensureSlot(std::size_t index)
        {
            // New slots start with an empty allocator, which is exactly the
            // state every other slot is in at the start of a frame.  The
            // buffer is created at its final size directly.
            while (_slots.size() <= index)
            {
                _slots.push_back(VAO::create(_ctx, _slotSize));
            }
        }

    } // namespace webgpu
} // namespace tl
