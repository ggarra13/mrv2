// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Observer.h>
#include <tlCore/Util.h>

#include <functional>
#include <memory>
#include <vector>

namespace tl
{
    namespace observer
    {
        template <typename T> class IValue;

        //! Value observer.
        template <typename T>
        class ValueObserver
            : public std::enable_shared_from_this<ValueObserver<T> >
        {
            TLRENDER_NON_COPYABLE(ValueObserver);

        protected:
            void _init(
                const std::weak_ptr<IValue<T> >&,
                const std::function<void(const T&)>&, CallbackAction);

            ValueObserver();

        public:
            ~ValueObserver();

            //! Create a new value observer.
            static std::shared_ptr<ValueObserver<T> > create(
                const std::weak_ptr<IValue<T> >&,
                const std::function<void(const T&)>&,
                CallbackAction = CallbackAction::Trigger);

            //! Execute the callback.
            void doCallback(const T&);

        private:
            std::function<void(const T&)> _callback;
            std::weak_ptr<IValue<T> > _value;
        };

        //! Base class for an observable value.
        template <typename T> class IValue
        {
        public:
            virtual ~IValue() = 0;

            //! Get the value.
            virtual const T& get() const = 0;

            //! Get the number of observers.
            std::size_t getObserversCount() const;

        protected:
            void _add(const std::weak_ptr<ValueObserver<T> >&);
            void _removeExpired();

            std::vector<std::weak_ptr<ValueObserver<T> > > _observers;

            friend class ValueObserver<T>;
        };

        //! Observable value.
        template <typename T> class Value : public IValue<T>
        {
            TLRENDER_NON_COPYABLE(Value);

        protected:
            Value();
            explicit Value(const T&);

        public:
            //! Create a new value.
            static std::shared_ptr<Value<T> > create();

            //! Create a new value.
            static std::shared_ptr<Value<T> > create(const T&);

            //! Set the value.
            void setAlways(const T&);

            //! Set the value only if it has changed.
            bool setIfChanged(const T&);

            const T& get() const override;

        private:
            T _value = T();
        };
    } // namespace observer
} // namespace tl

#include <tlCore/ValueObserverInline.h>
