// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/OCIOModel.h>

#include <QAbstractListModel>

namespace tl
{
    namespace play_qt
    {
        //! OpenColorIO input model.
        class OCIOInputModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIOInputModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIOInputModel();

            int
            rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant
            data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! OpenColorIO display model.
        class OCIODisplayModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIODisplayModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIODisplayModel();

            int
            rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant
            data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! OpenColorIO view model.
        class OCIOViewModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIOViewModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIOViewModel();

            int
            rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant
            data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! OpenColorIO look model.
        class OCIOLookModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIOLookModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIOLookModel();

            int
            rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant
            data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
