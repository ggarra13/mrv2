// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>
#include <vector>

namespace tl
{
    namespace memory
    {
        //! Least recently used (LRU) cache.
        template <typename T, typename U>
        class LRUCache
        {
        public:
            //! \name Size
            ///@{

            size_t getMax() const;
            size_t getSize() const;
            size_t getCount() const;
            float getPercentage() const;

            void setMax(size_t);

            ///@}

            //! \name Contents
            ///@{

            bool contains(const T& key) const;
            bool get(const T& key, U& value);
            bool touch(const T& key);

            void add(const T& key, const U& value, size_t size = 1);
            void remove(const T& key);
            void clear();

            std::vector<T> getKeys() const;
            std::vector<U> getValues() const;

            ///@}

        private:
            void _evict();

            size_t _max = 10000;
            size_t _size = 0; // running total of item sizes

            // LRU order: front = most-recently-used, back = least-recently-used.
            struct ListNode
            {
                ListNode(const T& key, const U& value, size_t size) :
                    key(key),
                    value(value),
                    size(size)
                    {}

                T      key;
                U      value;
                size_t size = 1;
            };
            std::list<ListNode> _list;

            // Maps key -> iterator into _list for O(1) lookup and splice.
            using Map = std::unordered_map<T, typename std::list<ListNode>::iterator>;
            Map _map;
        };
    } // namespace memory
} // namespace tl

#include <tlCore/LRUCacheInline.h>
