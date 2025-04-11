// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/OCIOModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play_qt
    {
        struct OCIOInputModel::Private
        {
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> >
                dataObserver;
        };

        OCIOInputModel::OCIOInputModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver =
                observer::ValueObserver<play::OCIOModelData>::create(
                    ocioModel->observeData(),
                    [this](const play::OCIOModelData& value)
                    {
                        beginResetModel();
                        _p->inputs = value.inputs;
                        _p->inputIndex = value.inputIndex;
                        endResetModel();
                    });
        }

        OCIOInputModel::~OCIOInputModel() {}

        int OCIOInputModel::rowCount(const QModelIndex& parent) const
        {
            return _p->inputs.size();
        }

        QVariant OCIOInputModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() && index.row() >= 0 &&
                index.row() < p.inputs.size() && index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(
                        QString::fromUtf8(p.inputs[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.inputIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.inputIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default:
                    break;
                }
            }
            return out;
        }

        struct OCIODisplayModel::Private
        {
            std::vector<std::string> displays;
            size_t displayIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> >
                dataObserver;
        };

        OCIODisplayModel::OCIODisplayModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver =
                observer::ValueObserver<play::OCIOModelData>::create(
                    ocioModel->observeData(),
                    [this](const play::OCIOModelData& value)
                    {
                        beginResetModel();
                        _p->displays = value.displays;
                        _p->displayIndex = value.displayIndex;
                        endResetModel();
                    });
        }

        OCIODisplayModel::~OCIODisplayModel() {}

        int OCIODisplayModel::rowCount(const QModelIndex& parent) const
        {
            return _p->displays.size();
        }

        QVariant
        OCIODisplayModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() && index.row() >= 0 &&
                index.row() < p.displays.size() && index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(
                        QString::fromUtf8(p.displays[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.displayIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.displayIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default:
                    break;
                }
            }
            return out;
        }

        struct OCIOViewModel::Private
        {
            std::vector<std::string> views;
            size_t viewIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> >
                dataObserver;
        };

        OCIOViewModel::OCIOViewModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver =
                observer::ValueObserver<play::OCIOModelData>::create(
                    ocioModel->observeData(),
                    [this](const play::OCIOModelData& value)
                    {
                        beginResetModel();
                        _p->views = value.views;
                        _p->viewIndex = value.viewIndex;
                        endResetModel();
                    });
        }

        OCIOViewModel::~OCIOViewModel() {}

        int OCIOViewModel::rowCount(const QModelIndex& parent) const
        {
            return _p->views.size();
        }

        QVariant OCIOViewModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() && index.row() >= 0 &&
                index.row() < p.views.size() && index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(
                        QString::fromUtf8(p.views[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.viewIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.viewIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default:
                    break;
                }
            }
            return out;
        }

        struct OCIOLookModel::Private
        {
            std::vector<std::string> looks;
            size_t lookIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> >
                dataObserver;
        };

        OCIOLookModel::OCIOLookModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver =
                observer::ValueObserver<play::OCIOModelData>::create(
                    ocioModel->observeData(),
                    [this](const play::OCIOModelData& value)
                    {
                        beginResetModel();
                        _p->looks = value.looks;
                        _p->lookIndex = value.lookIndex;
                        endResetModel();
                    });
        }

        OCIOLookModel::~OCIOLookModel() {}

        int OCIOLookModel::rowCount(const QModelIndex& parent) const
        {
            return _p->looks.size();
        }

        QVariant OCIOLookModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() && index.row() >= 0 &&
                index.row() < p.looks.size() && index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(
                        QString::fromUtf8(p.looks[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.lookIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.lookIndex)
                    {
                        out.setValue(QBrush(qApp->palette().color(
                            QPalette::ColorRole::HighlightedText)));
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
