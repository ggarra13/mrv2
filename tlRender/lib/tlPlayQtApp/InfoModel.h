// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <QAbstractTableModel>

#include <memory>

namespace tl
{
    namespace io
    {
        struct Info;
    }

    namespace play_qt
    {
        //! Tags model.
        class TagsModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            TagsModel(QObject* parent = nullptr);

            virtual ~TagsModel();

            //! Set the tags.
            void setTags(const image::Tags&);

            int
            rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(
                const QModelIndex& parent = QModelIndex()) const override;
            QVariant
            data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            QVariant headerData(
                int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
