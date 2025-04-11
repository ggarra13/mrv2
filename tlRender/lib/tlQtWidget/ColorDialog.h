// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Util.h>

#include <QDialog>

namespace tl
{
    namespace qtwidget
    {
        //! Color picker dialog.
        class ColorDialog : public QDialog
        {
            Q_OBJECT

        public:
            ColorDialog(
                const image::Color4f& = image::Color4f(),
                QWidget* parent = nullptr);

            virtual ~ColorDialog();

            //! Get the color.
            const image::Color4f& color() const;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
