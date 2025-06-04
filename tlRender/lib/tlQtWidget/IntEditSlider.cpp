// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/IntEditSlider.h>

#include <tlQtWidget/Util.h>

#include <QBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        struct IntEditSlider::Private
        {
            math::IntRange range = math::IntRange(0, 100);
            int value = 0;
            int defaultValue = -1;
            int singleStep = 1;
            int pageStep = 10;
            Qt::Orientation orientation = Qt::Horizontal;
            QSpinBox* spinBox = nullptr;
            QSlider* slider = nullptr;
            QToolButton* defaultValueButton = nullptr;
            QBoxLayout* layout = nullptr;
        };

        IntEditSlider::IntEditSlider(
            Qt::Orientation orientation, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.spinBox = new QSpinBox;
            p.spinBox->setFont(QFont("Noto Mono"));

            p.defaultValueButton = new QToolButton;
            p.defaultValueButton->setAutoRaise(true);
            p.defaultValueButton->setIcon(QIcon(":/Icons/Reset.svg"));
            p.defaultValueButton->setToolTip(tr("Reset to the default value"));

            _layoutUpdate();
            _widgetUpdate();

            connect(
                p.spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    _p->value = value;
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });

            connect(
                p.defaultValueButton, &QToolButton::clicked,
                [this] { setValue(_p->defaultValue); });
        }

        IntEditSlider::~IntEditSlider() {}

        const math::IntRange& IntEditSlider::range() const
        {
            return _p->range;
        }

        int IntEditSlider::value() const
        {
            return _p->value;
        }

        int IntEditSlider::defaultValue() const
        {
            return _p->defaultValue;
        }

        int IntEditSlider::singleStep() const
        {
            return _p->singleStep;
        }

        int IntEditSlider::pageStep() const
        {
            return _p->pageStep;
        }

        Qt::Orientation IntEditSlider::orientation() const
        {
            return _p->orientation;
        }

        void IntEditSlider::setRange(const math::IntRange& value)
        {
            TLRENDER_P();
            if (value == p.range)
                return;
            p.range = value;
            _widgetUpdate();
            Q_EMIT rangeChanged(p.range);
        }

        void IntEditSlider::setValue(int value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
            Q_EMIT valueChanged(p.value);
        }

        void IntEditSlider::setDefaultValue(int value)
        {
            TLRENDER_P();
            if (value == p.defaultValue)
                return;
            p.defaultValue = value;
            _widgetUpdate();
        }

        void IntEditSlider::setSingleStep(int value)
        {
            TLRENDER_P();
            if (value == p.singleStep)
                return;
            p.singleStep = value;
            _widgetUpdate();
        }

        void IntEditSlider::setPageStep(int value)
        {
            TLRENDER_P();
            if (value == p.pageStep)
                return;
            p.pageStep = value;
            _widgetUpdate();
        }

        void IntEditSlider::setOrientation(Qt::Orientation value)
        {
            TLRENDER_P();
            if (value == p.orientation)
                return;
            p.orientation = value;
            _layoutUpdate();
        }

        void IntEditSlider::_layoutUpdate()
        {
            TLRENDER_P();

            if (p.layout)
            {
                p.layout->removeWidget(p.spinBox);
                p.layout->removeWidget(p.slider);
                p.layout->removeWidget(p.defaultValueButton);
            }
            delete p.slider;
            delete p.layout;

            p.slider = new QSlider(p.orientation);
            switch (p.orientation)
            {
            case Qt::Horizontal:
                p.layout = new QHBoxLayout;
                break;
            case Qt::Vertical:
                p.layout = new QVBoxLayout;
                break;
            default:
                break;
            }
            p.layout->setContentsMargins(0, 0, 0, 0);

            p.layout->addWidget(p.spinBox);
            p.layout->addWidget(p.slider, 1);
            p.layout->addWidget(p.defaultValueButton);
            setLayout(p.layout);

            connect(
                p.slider, &QSlider::valueChanged,
                [this](int value)
                {
                    _p->value = value;
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });
        }

        void IntEditSlider::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.spinBox);
                p.spinBox->setRange(p.range.getMin(), p.range.getMax());
                p.spinBox->setValue(p.value);
                p.spinBox->setSingleStep(p.singleStep);
            }
            {
                QSignalBlocker blocker(p.slider);
                p.slider->setRange(p.range.getMin(), p.range.getMax());
                p.slider->setValue(p.value);
                p.slider->setSingleStep(p.singleStep);
                p.slider->setPageStep(p.pageStep);
            }
            p.defaultValueButton->setVisible(p.range.contains(p.defaultValue));
            p.defaultValueButton->setEnabled(p.value != p.defaultValue);
        }
    } // namespace qtwidget
} // namespace tl
