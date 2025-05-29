// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Observer.h>
#include <tlCore/Util.h>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace tl
{
    namespace observer
    {
        template <typename T, typename U> class IMap;

        //! Map observer.
        template <typename T, typename U>
        class MapObserver
            : public std::enable_shared_from_this<MapObserver<T, U> >
        {
            TLRENDER_NON_COPYABLE(MapObserver);

            void _init(
                const std::weak_ptr<IMap<T, U> >&,
                const std::function<void(const std::map<T, U>&)>&,
                CallbackAction);

            MapObserver();

        public:
            ~MapObserver();

            //! Create a new map observer.
            static std::shared_ptr<MapObserver<T, U> > create(
                const std::weak_ptr<IMap<T, U> >&,
                const std::function<void(const std::map<T, U>&)>&,
                CallbackAction = CallbackAction::Trigger);

            //! Execute the callback.
            void doCallback(const std::map<T, U>&);

        private:
            std::function<void(const std::map<T, U>&)> _callback;
            std::weak_ptr<IMap<T, U> > _value;
        };

        //! Base class for an observable map.
        template <typename T, typename U> class IMap
        {
        public:
            virtual ~IMap() = 0;

            //! Get the map.
            virtual const std::map<T, U>& get() const = 0;

            //! Get the map size.
            virtual std::size_t getSize() const = 0;

            //! Get whether the map is empty.
            virtual bool isEmpty() const = 0;

            //! Does the given key exist?
            virtual bool hasKey(const T&) = 0;

            //! Get a map item.
            virtual const U& getItem(const T&) const = 0;

            //! Get the number of observers.
            std::size_t getObserversCount() const;

        protected:
            void _add(const std::weak_ptr<MapObserver<T, U> >&);
            void _removeExpired();

            std::vector<std::weak_ptr<MapObserver<T, U> > > _observers;

            friend MapObserver<T, U>;
        };

        //! Observable map.
        template <typename T, typename U> class Map : public IMap<T, U>
        {
            TLRENDER_NON_COPYABLE(Map);

            Map();
            explicit Map(const std::map<T, U>&);

        public:
            //! Create a new map.
            static std::shared_ptr<Map<T, U> > create();

            //! Create a new map.
            static std::shared_ptr<Map<T, U> > create(const std::map<T, U>&);

            //! Set the map.
            void setAlways(const std::map<T, U>&);

            //! Set the map only if it has changed.
            bool setIfChanged(const std::map<T, U>&);

            //! Clear the map.
            void clear();

            //! Set a map item.
            void setItem(const T&, const U&);

            //! Set a map item only if it has changed.
            void setItemOnlyIfChanged(const T&, const U&);

            const std::map<T, U>& get() const override;
            std::size_t getSize() const override;
            bool isEmpty() const override;
            bool hasKey(const T&) override;
            const U& getItem(const T&) const override;

        private:
            std::map<T, U> _value;
        };
    } // namespace observer
} // namespace tl

#include <tlCore/MapObserverInline.h>
