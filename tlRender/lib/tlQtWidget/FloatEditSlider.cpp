// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FloatEditSlider.h>

#include <tlQtWidget/Util.h>

#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        namespace
        {
            const size_t steps = 10000;
        }

        struct FloatEditSlider::Private
        {
            math::FloatRange range = math::FloatRange(0.F, 1.F);
            float value = 0.F;
            float defaultValue = -1.F;
            float singleStep = .01F;
            float pageStep = .1F;
            Qt::Orientation orientation = Qt::Horizontal;
            QDoubleSpinBox* spinBox = nullptr;
            QSlider* slider = nullptr;
            QToolButton* defaultValueButton = nullptr;
            QBoxLayout* layout = nullptr;
        };

        FloatEditSlider::FloatEditSlider(
            Qt::Orientation orientation, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.spinBox = new QDoubleSpinBox;
            p.spinBox->setFont(QFont("Noto Mono"));

            p.defaultValueButton = new QToolButton;
            p.defaultValueButton->setAutoRaise(true);
            p.defaultValueButton->setIcon(QIcon(":/Icons/Reset.svg"));
            p.defaultValueButton->setToolTip(tr("Reset to the default value"));

            _layoutUpdate();
            _widgetUpdate();

            connect(
                p.spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    _p->value = value;
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });

            connect(
                p.defaultValueButton, &QToolButton::clicked,
                [this] { setValue(_p->defaultValue); });
        }

        FloatEditSlider::~FloatEditSlider() {}

        const math::FloatRange& FloatEditSlider::range() const
        {
            return _p->range;
        }

        float FloatEditSlider::value() const
        {
            return _p->value;
        }

        float FloatEditSlider::defaultValue() const
        {
            return _p->defaultValue;
        }

        float FloatEditSlider::singleStep() const
        {
            return _p->singleStep;
        }

        float FloatEditSlider::pageStep() const
        {
            return _p->pageStep;
        }

        Qt::Orientation FloatEditSlider::orientation() const
        {
            return _p->orientation;
        }

        void FloatEditSlider::setRange(const math::FloatRange& value)
        {
            TLRENDER_P();
            if (value == p.range)
                return;
            p.range = value;
            _widgetUpdate();
            Q_EMIT rangeChanged(p.range);
        }

        void FloatEditSlider::setValue(float value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
            Q_EMIT valueChanged(p.value);
        }

        void FloatEditSlider::setDefaultValue(float value)
        {
            TLRENDER_P();
            if (value == p.defaultValue)
                return;
            p.defaultValue = value;
            _widgetUpdate();
        }

        void FloatEditSlider::setSingleStep(float value)
        {
            TLRENDER_P();
            if (value == p.singleStep)
                return;
            p.singleStep = value;
            _widgetUpdate();
        }

        void FloatEditSlider::setPageStep(float value)
        {
            TLRENDER_P();
            if (value == p.pageStep)
                return;
            p.pageStep = value;
            _widgetUpdate();
        }

        void FloatEditSlider::setOrientation(Qt::Orientation value)
        {
            TLRENDER_P();
            if (value == p.orientation)
                return;
            p.orientation = value;
            _layoutUpdate();
        }

        void FloatEditSlider::_layoutUpdate()
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
                    const float n = value / static_cast<float>(steps);
                    _p->value = n * (_p->range.getMax() - _p->range.getMin()) +
                                _p->range.getMin();
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });
        }

        void FloatEditSlider::_widgetUpdate()
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
                p.slider->setRange(0, steps);
                const float n = (p.value - p.range.getMin()) /
                                (p.range.getMax() - p.range.getMin());
                p.slider->setValue(n * steps);
                p.slider->setSingleStep(p.singleStep);
                p.slider->setPageStep(p.pageStep);
            }
            p.defaultValueButton->setVisible(p.range.contains(p.defaultValue));
            p.defaultValueButton->setEnabled(p.value != p.defaultValue);
        }
    } // namespace qtwidget
} // namespace tl
