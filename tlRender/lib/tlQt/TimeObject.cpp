// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQt/TimeObject.h>

#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <QDataStream>

#include <array>

namespace tl
{
    namespace qt
    {
        TimeObject::TimeObject(
            const std::shared_ptr<timeline::TimeUnitsModel>& model,
            QObject* parent) :
            QObject(parent),
            _model(model)
        {
        }

        timeline::TimeUnits TimeObject::timeUnits() const
        {
            return _model->getTimeUnits();
        }

        void TimeObject::setTimeUnits(timeline::TimeUnits value)
        {
            const timeline::TimeUnits units = _model->getTimeUnits();
            _model->setTimeUnits(value);
            if (units != _model->getTimeUnits())
            {
                Q_EMIT timeUnitsChanged(_model->getTimeUnits());
            }
        }

        QDataStream&
        operator<<(QDataStream& ds, const timeline::TimeUnits& value)
        {
            ds << static_cast<qint32>(value);
            return ds;
        }

        QDataStream& operator>>(QDataStream& ds, timeline::TimeUnits& value)
        {
            qint32 tmp = 0;
            ds >> tmp;
            value = static_cast<timeline::TimeUnits>(tmp);
            return ds;
        }

    } // namespace qt
} // namespace tl
