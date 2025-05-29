// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ColorDialog.h>

#include <tlQtWidget/ColorWidget.h>

#include <QBoxLayout>
#include <QDialogButtonBox>

namespace tl
{
    namespace qtwidget
    {
        struct ColorDialog::Private
        {
            ColorWidget* colorWidget = nullptr;
        };

        ColorDialog::ColorDialog(const image::Color4f& color, QWidget* parent) :
            QDialog(parent),
            _p(new Private)
        {
            TLRENDER_P();

            setWindowTitle(tr("Color Picker"));

            p.colorWidget = new ColorWidget;
            p.colorWidget->setColor(color);

            auto buttonBox = new QDialogButtonBox;
            buttonBox->addButton(QDialogButtonBox::Ok);
            buttonBox->addButton(QDialogButtonBox::Cancel);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.colorWidget);
            layout->addWidget(buttonBox);
            setLayout(layout);

            connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
            connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
        }

        ColorDialog::~ColorDialog() {}

        const image::Color4f& ColorDialog::color() const
        {
            return _p->colorWidget->color();
        }
    } // namespace qtwidget
} // namespace tl
