// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <map>
#include <vector>

namespace mrv
{

    template <typename Key, typename Value> class OrderedMap
    {
    public:
        void insert(const Key& key, const Value& value)
        {
            // Check if the key already exists in the map
            if (map_.count(key) == 0)
            {
                // Add the key to the vector
                keys_.push_back(key);
            }

            // Insert the key-value pair into the map
            map_[key] = value;
        }

        const Value& at(const Key& key) const { return map_.at(key); }

        Value& operator[](const Key& key)
        {
            // Check if the key already exists in the map
            if (map_.count(key) == 0)
            {
                // Add the key to the vector
                keys_.push_back(key);
            }

            // Access or insert the key-value pair into the map
            return map_[key];
        }

        std::size_t size() const { return keys_.size(); }

        // Iterate over the elements in the order of insertion
        using iterator = typename std::vector<Key>::iterator;
        using const_iterator = typename std::vector<Key>::const_iterator;

        iterator begin() { return keys_.begin(); }

        const_iterator begin() const { return keys_.begin(); }

        iterator end() { return keys_.end(); }

        const_iterator end() const { return keys_.end(); }

    private:
        std::map<Key, Value> map_;
        std::vector<Key> keys_;
    };

} // namespace mrv
