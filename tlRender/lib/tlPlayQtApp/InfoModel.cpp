// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/InfoModel.h>

#include <tlIO/IO.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_qt
    {
        struct TagsModel::Private
        {
            image::Tags tags;
            QList<QPair<QString, QString> > items;
        };

        TagsModel::TagsModel(QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {
        }

        TagsModel::~TagsModel() {}

        void TagsModel::setTags(const image::Tags& value)
        {
            TLRENDER_P();
            if (value == p.tags)
                return;
            p.tags = value;
            beginResetModel();
            p.items.clear();
            for (const auto& i : p.tags)
            {
                p.items.push_back(QPair<QString, QString>(
                    QString::fromUtf8(i.first.c_str()),
                    QString::fromUtf8(i.second.c_str())));
            }
            endResetModel();
        }

        int TagsModel::rowCount(const QModelIndex& parent) const
        {
            return _p->items.count();
        }

        int TagsModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        QVariant TagsModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() && index.row() >= 0 &&
                index.row() < p.items.count() && index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 0:
                        out.setValue(p.items.at(index.row()).first);
                        break;
                    case 1:
                        out.setValue(p.items.at(index.row()).second);
                        break;
                    }
                    break;
                }
                case Qt::ToolTipRole:
                    out.setValue(QString("%1: %2")
                                     .arg(p.items.at(index.row()).first)
                                     .arg(p.items.at(index.row()).second));
                    break;
                default:
                    break;
                }
            }
            return out;
        }

        QVariant TagsModel::headerData(
            int section, Qt::Orientation orientation, int role) const
        {
            QVariant out;
            if (Qt::Horizontal == orientation)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0:
                        out = tr("Name");
                        break;
                    case 1:
                        out = tr("Value");
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            return out;
        }
    } // namespace play_qt
} // namespace tl
