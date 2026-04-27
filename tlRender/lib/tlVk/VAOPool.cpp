// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "VAOPool.h"

#include <tlVk/Mesh.h>

#include <stdexcept>

namespace tl
{
    namespace vlk
    {
        
        // ------------------------------------------------------------------
        //  VAOPool
        // ------------------------------------------------------------------

        VAOPool::VAOPool(Fl_Vk_Context& ctx, VkDeviceSize slotSize)
            : _ctx(ctx)
            , _slotSize(slotSize)
        {
            // Pre-allocate the first slot so the pool is ready to use
            // immediately after construction without a bind() call.
            _ensureSlot(0);
        }

        std::shared_ptr<VAOPool>
        VAOPool::create(Fl_Vk_Context& ctx, VkDeviceSize slotSize)
        {
            // Can't use make_shared – constructor is private.
            return std::shared_ptr<VAOPool>(new VAOPool(ctx, slotSize));
        }

        void VAOPool::bind(uint32_t frameIndex)
        {
            _frameIndex  = frameIndex;
            _currentSlot = 0;

            // Advance every existing slot to the new frame.  VAO::bind()
            // resets relativeOffset to 0 whenever frameIndex changes, which
            // is exactly what we need at the start of each frame.
            for (auto& slot : _slots)
                slot->bind(frameIndex);
        }

        VAOAllocation VAOPool::upload(const std::vector<uint8_t>& data,
                                      std::size_t                 vertexCount)
        {
            // Fast path: current slot has room.
            if (!_slots[_currentSlot]->canFit(data.size()))
            {
                ++_currentSlot;
                _ensureSlot(_currentSlot);

                // If it still can't fit the upload is larger than one slot's
                // per-frame region, which is an irrecoverable configuration
                // error (a single draw call can't span two VkBuffers).
                if (!_slots[_currentSlot]->canFit(data.size()))
                {
                    throw std::runtime_error(
                        "VAOPool: single mesh upload exceeds one slot's "
                        "per-frame region.  Increase slotSize.");
                }
            }

            VAOAllocation alloc;
            alloc.poolIndex   = _currentSlot;
            alloc.offset      = static_cast<VkDeviceSize>(
                                    _slots[_currentSlot]->upload(data));
            alloc.vertexCount = vertexCount;
            return alloc;
        }

        VAOAllocation VAOPool::upload(const std::shared_ptr<VBO>& vbo)
        {
            return upload(vbo->getData(), vbo->getSize());
        }

        void VAOPool::draw(VkCommandBuffer& cmd, const VAOAllocation& alloc)
        {
            _slots[alloc.poolIndex]->draw(cmd, alloc.vertexCount, alloc.offset);
        }

        std::size_t VAOPool::slotCount() const
        {
            return _slots.size();
        }

        void VAOPool::_ensureSlot(std::size_t index)
        {
            if (index < _slots.size())
                return; // already exists

            // Grow the vector to the required size.
            while (_slots.size() <= index)
            {
                auto vao = VAO::create(_ctx);
                vao->setMemorySize(_slotSize);

                // Sync the new slot to the current frame.  VAO starts with
                // frameIndex=0 / relativeOffset=0.  If we are already past
                // frame 0 we need to call bind() to align it; if we are on
                // frame 0 the VAO is already in the right state.
                if (_frameIndex != 0)
                    vao->bind(_frameIndex);

                _slots.push_back(std::move(vao));
            }
        }

    } // namespace vlk
} // namespace tl
