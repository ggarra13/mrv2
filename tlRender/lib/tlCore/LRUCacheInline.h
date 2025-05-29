// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace memory
    {
        template <typename T, typename U>
        inline std::size_t LRUCache<T, U>::getMax() const
        {
            return _max;
        }

        template <typename T, typename U>
        inline std::size_t LRUCache<T, U>::getSize() const
        {
            size_t out = 0;
            for (const auto& i : _map)
            {
                out += i.second.second;
            }
            return out;
        }

        template <typename T, typename U>
        inline std::size_t LRUCache<T, U>::getCount() const
        {
            return _map.size();
        }

        template <typename T, typename U>
        inline float LRUCache<T, U>::getPercentage() const
        {
            return getSize() / static_cast<float>(_max) * 100.F;
        }

        template <typename T, typename U>
        inline void LRUCache<T, U>::setMax(std::size_t value)
        {
            if (value == _max)
                return;
            _max = value;
            _maxUpdate();
        }

        template <typename T, typename U>
        inline bool LRUCache<T, U>::contains(const T& key) const
        {
            return _map.find(key) != _map.end();
        }

        template <typename T, typename U>
        inline bool LRUCache<T, U>::get(const T& key, U& value) const
        {
            auto i = _map.find(key);
            if (i != _map.end())
            {
                value = i->second.first;
                auto j = _counts.find(key);
                if (j != _counts.end())
                {
                    ++_counter;
                    j->second = _counter;
                }
                return true;
            }
            return i != _map.end();
        }

        template <typename T, typename U>
        inline void
        LRUCache<T, U>::add(const T& key, const U& value, size_t size)
        {
            _map[key] = std::make_pair(value, size);
            ++_counter;
            _counts[key] = _counter;
            _maxUpdate();
        }

        template <typename T, typename U>
        inline void LRUCache<T, U>::remove(const T& key)
        {
            const auto i = _map.find(key);
            if (i != _map.end())
            {
                _map.erase(i);
            }
            const auto j = _counts.find(key);
            if (j != _counts.end())
            {
                _counts.erase(j);
            }
            _maxUpdate();
        }

        template <typename T, typename U> inline void LRUCache<T, U>::clear()
        {
            _map.clear();
        }

        template <typename T, typename U>
        inline std::vector<T> LRUCache<T, U>::getKeys() const
        {
            std::vector<T> out;
            for (const auto& i : _map)
            {
                out.push_back(i.first);
            }
            return out;
        }

        template <typename T, typename U>
        inline std::vector<U> LRUCache<T, U>::getValues() const
        {
            std::vector<U> out;
            for (const auto& i : _map)
            {
                out.push_back(i.second.first);
            }
            return out;
        }

        template <typename T, typename U>
        inline void LRUCache<T, U>::_maxUpdate()
        {
            size_t size = getSize();
            if (size > _max)
            {
                std::map<int64_t, T> sorted;
                for (const auto& i : _counts)
                {
                    sorted[i.second] = i.first;
                }
                while (getSize() > _max)
                {
                    auto begin = sorted.begin();
                    auto i = _map.find(begin->second);
                    if (i != _map.end())
                    {
                        _map.erase(i);
                    }
                    auto j = _counts.find(begin->second);
                    if (j != _counts.end())
                    {
                        _counts.erase(j);
                    }
                    sorted.erase(begin);
                }
            }
        }
    } // namespace memory
} // namespace tl
