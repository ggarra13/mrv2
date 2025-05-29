// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace observer
    {
        template <typename T>
        inline void ListObserver<T>::_init(
            const std::weak_ptr<IList<T> >& value,
            const std::function<void(const std::vector<T>&)>& callback,
            CallbackAction action)
        {
            _value = value;
            _callback = callback;
            if (auto value = _value.lock())
            {
                value->_add(ListObserver<T>::shared_from_this());
                if (CallbackAction::Trigger == action)
                {
                    _callback(value->get());
                }
            }
        }

        template <typename T> inline ListObserver<T>::ListObserver() {}

        template <typename T> inline ListObserver<T>::~ListObserver()
        {
            if (auto value = _value.lock())
            {
                value->_removeExpired();
            }
        }

        template <typename T>
        inline std::shared_ptr<ListObserver<T> > ListObserver<T>::create(
            const std::weak_ptr<IList<T> >& value,
            const std::function<void(const std::vector<T>&)>& callback,
            CallbackAction action)
        {
            std::shared_ptr<ListObserver<T> > out(new ListObserver<T>);
            out->_init(value, callback, action);
            return out;
        }

        template <typename T>
        inline void ListObserver<T>::doCallback(const std::vector<T>& value)
        {
            _callback(value);
        }

        template <typename T> inline IList<T>::~IList() {}

        template <typename T>
        inline std::size_t IList<T>::getObserversCount() const
        {
            return _observers.size();
        }

        template <typename T>
        inline void
        IList<T>::_add(const std::weak_ptr<ListObserver<T> >& observer)
        {
            _observers.push_back(observer);
        }

        template <typename T> inline void IList<T>::_removeExpired()
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

        template <typename T> inline List<T>::List() {}

        template <typename T>
        inline List<T>::List(const std::vector<T>& value) :
            _value(value)
        {
        }

        template <typename T> inline std::shared_ptr<List<T> > List<T>::create()
        {
            return std::shared_ptr<List<T> >(new List<T>);
        }

        template <typename T>
        inline std::shared_ptr<List<T> >
        List<T>::create(const std::vector<T>& value)
        {
            return std::shared_ptr<List<T> >(new List<T>(value));
        }

        template <typename T>
        inline void List<T>::setAlways(const std::vector<T>& value)
        {
            _value = value;
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T>
        inline bool List<T>::setIfChanged(const std::vector<T>& value)
        {
            if (value == _value)
                return false;
            _value = value;
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
            return true;
        }

        template <typename T> inline void List<T>::clear()
        {
            if (_value.size())
            {
                _value.clear();
                for (const auto& i : IList<T>::_observers)
                {
                    if (auto observer = i.lock())
                    {
                        observer->doCallback(_value);
                    }
                }
            }
        }

        template <typename T>
        inline void List<T>::setItem(std::size_t index, const T& value)
        {
            _value[index] = value;
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T>
        inline void
        List<T>::setItemOnlyIfChanged(std::size_t index, const T& value)
        {
            if (value == _value[index])
                return;
            _value[index] = value;
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T> inline void List<T>::pushBack(const T& value)
        {
            _value.push_back(value);
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T> inline void List<T>::removeItem(std::size_t index)
        {
            _value.erase(_value.begin() + index);
            for (const auto& i : IList<T>::_observers)
            {
                if (auto observer = i.lock())
                {
                    observer->doCallback(_value);
                }
            }
        }

        template <typename T> inline const std::vector<T>& List<T>::get() const
        {
            return _value;
        }

        template <typename T> inline std::size_t List<T>::getSize() const
        {
            return _value.size();
        }

        template <typename T> inline bool List<T>::isEmpty() const
        {
            return _value.empty();
        }

        template <typename T>
        inline const T& List<T>::getItem(std::size_t index) const
        {
            return _value[index];
        }

        template <typename T>
        inline bool List<T>::contains(const T& value) const
        {
            const auto i = std::find_if(
                _value.begin(), _value.end(),
                [value](const T& other) { return value == other; });
            return i != _value.end();
        }

        template <typename T>
        inline std::size_t List<T>::indexOf(const T& value) const
        {
            const auto i = std::find_if(
                _value.begin(), _value.end(),
                [value](const T& other) { return value == other; });
            return i != _value.end() ? i - _value.begin() : invalidListIndex;
        }
    } // namespace observer
} // namespace tl
