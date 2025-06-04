// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace observer
    {
        template <typename T>
        inline void ValueObserver<T>::_init(
            const std::weak_ptr<IValue<T> >& value,
            const std::function<void(const T&)>& callback,
            CallbackAction action)
        {
            _value = value;
            _callback = callback;
            if (auto value = _value.lock())
            {
                value->_add(ValueObserver<T>::shared_from_this());
                if (CallbackAction::Trigger == action)
                {
                    _callback(value->get());
                }
            }
        }

        template <typename T> inline ValueObserver<T>::ValueObserver() {}

        template <typename T> inline ValueObserver<T>::~ValueObserver()
        {
            if (auto value = _value.lock())
            {
                value->_removeExpired();
            }
        }

        template <typename T>
        inline std::shared_ptr<ValueObserver<T> > ValueObserver<T>::create(
            const std::weak_ptr<IValue<T> >& value,
            const std::function<void(const T&)>& callback,
            CallbackAction action)
        {
            std::shared_ptr<ValueObserver<T> > out(new ValueObserver<T>);
            out->_init(value, callback, action);
            return out;
        }

        template <typename T>
        inline void ValueObserver<T>::doCallback(const T& value)
        {
            _callback(value);
        }

        template <typename T> inline IValue<T>::~IValue() {}

        template <typename T>
        inline std::size_t IValue<T>::getObserversCount() const
        {
            return _observers.size();
        }

        template <typename T>
        inline void
        IValue<T>::_add(const std::weak_ptr<ValueObserver<T> >& observer)
        {
            _observers.push_back(observer);
        }

        template <typename T> inline void IValue<T>::_removeExpired()
        {
            auto i = _observers.begin();
            while (i != _observers.end())
            {
                if (i->expired())
                {
                    i = _observers.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        template <typename T> inline Value<T>::Value() {}

        template <typename T>
        inline Value<T>::Value(const T& value) :
            _value(value)
        {
        }

        template <typename T>
        inline std::shared_ptr<Value<T> > Value<T>::create()
        {
            return std::shared_ptr<Value<T> >(new Value<T>);
        }

        template <typename T>
        inline std::shared_ptr<Value<T> > Value<T>::create(const T& value)
        {
            return std::shared_ptr<Value<T> >(new Value<T>(value));
        }

        template <typename T> inline void Value<T>::setAlways(const T& value)
        {
            _value = value;
            for (const auto& i : IValue<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T> inline bool Value<T>::setIfChanged(const T& value)
        {
            if (value == _value)
                return false;
            _value = value;
            for (const auto& i : IValue<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
            return true;
        }

        template <typename T> inline const T& Value<T>::get() const
        {
            return _value;
        }
    } // namespace observer
} // namespace tl
