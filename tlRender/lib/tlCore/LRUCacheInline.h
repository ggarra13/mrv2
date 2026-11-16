// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace memory
    {
        template <typename T, typename U>
        inline size_t LRUCache<T, U>::getMax() const
        {
            return _max;
        }

        template <typename T, typename U>
        inline size_t LRUCache<T, U>::getSize() const
        {
            return _size;
        }

        template <typename T, typename U>
        inline size_t LRUCache<T, U>::getCount() const
        {
            return _map.size();
        }

        template <typename T, typename U>
        inline float LRUCache<T, U>::getPercentage() const
        {
            return _max > 0 ?
                (_size / static_cast<float>(_max) * 100.F) :
                0.F;
        }

        template <typename T, typename U>
        inline void LRUCache<T, U>::setMax(std::size_t value)
        {
            if (value == _max)
                return;
            _max = value;
            _evict();
        }

        template <typename T, typename U>
        inline bool LRUCache<T, U>::contains(const T& key) const
        {
            return _map.find(key) != _map.end();
        }

        template <typename T, typename U>
        inline bool LRUCache<T, U>::get(const T& key, U& value)
        {
            auto i = _map.find(key);
            if (i == _map.end())
                return false;
            if (i->second != _list.begin())
            {
                // Move to front (most-recently-used).
                _list.splice(_list.begin(), _list, i->second);
            }
            value = i->second->value;
            return true;
        }

        template<typename T, typename U>
        inline bool LRUCache<T, U>::touch(const T& key)
        {
            const auto i = _map.find(key);
            if (i == _map.end())
                return false;
            if (i->second != _list.begin())
            {
                _list.splice(_list.begin(), _list, i->second);
            }
            return true;
        }

        template<typename T, typename U>
        inline void LRUCache<T, U>::add(const T& key, const U& value, size_t size)
        {
            if (const auto i = _map.find(key); i != _map.end())
            {
                // Update existing entry: adjust running size, move to front.
                _size -= i->second->size;
                i->second->value = value;
                i->second->size = size;
                _size += size;
                if (i->second != _list.begin())
                {
                    _list.splice(_list.begin(), _list, i->second);
                }
            }
            else
            {
                _list.emplace_front(key, value, size);
                _map[key] = _list.begin();
                _size += size;
            }
            _evict();
        }

        template<typename T, typename U>
        inline void LRUCache<T, U>::remove(const T& key)
        {
            if (const auto i = _map.find(key); i != _map.end())
            {
                _size -= i->second->size;
                _list.erase(i->second);
                _map.erase(i);
            }
        }

        template<typename T, typename U>
        inline void LRUCache<T, U>::clear()
        {
            _list.clear();
            _map.clear();
            _size = 0;
        }

        template<typename T, typename U>
        inline std::vector<T> LRUCache<T, U>::getKeys() const
        {
            std::vector<T> out;
            out.reserve(_map.size());
            for (const auto& node : _list)
            {
                out.push_back(node.key);
            }
            return out;
        }

        template<typename T, typename U>
        inline std::vector<U> LRUCache<T, U>::getValues() const
        {
            std::vector<U> out;
            out.reserve(_map.size());
            for (const auto& node : _list)
            {
                out.push_back(node.value);
            }
            return out;
        }

        template<typename T, typename U>
        inline void LRUCache<T, U>::_evict()
        {
            // Remove from the back (least-recently-used) until within budget.
            while (_size > _max && !_list.empty())
            {
                const auto& lru = _list.back();
                _size -= lru.size;
                _map.erase(lru.key);
                _list.pop_back();
            }
        }
    } // namespace memory
} // namespace tl
