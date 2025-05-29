// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/Viewport.h>

#include <tlCore/StringFormat.h>

#include <QPainter>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const int margin = 5;
            const int fontSize = 12;
        } // namespace

        struct Viewport::Private
        {
            bool hud = false;
            double fps = 0.0;
            size_t droppedFrames = 0;
            std::vector<std::string> text;
        };

        Viewport::Viewport(
            const std::shared_ptr<system::Context>& context, QWidget* parent) :
            TimelineViewport(context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            connect(
                this, &TimelineViewport::fpsChanged,
                [this](double value)
                {
                    _p->fps = value;
                    _textUpdate();
                });

            connect(
                this, &TimelineViewport::droppedFramesChanged,
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _textUpdate();
                });
        }

        Viewport::~Viewport() {}

        bool Viewport::hasHUD() const
        {
            return _p->hud;
        }

        void Viewport::setHUD(bool value)
        {
            TLRENDER_P();
            if (value == p.hud)
                return;
            p.hud = value;
            Q_EMIT hudChanged(p.hud);
            update();
        }

        void Viewport::paintGL()
        {
            TimelineViewport::paintGL();
            TLRENDER_P();
            if (p.hud)
            {
                QPainter painter(this);
                const math::Box2i g(0, 0, width(), height());
                const math::Box2i g2 = g.margin(-margin);
                QFont font("fixed");
                font.setPixelSize(fontSize);
                painter.setFont(font);
                painter.setPen(Qt::white);
                QFontMetrics fm(font);
                int x = g2.min.x;
                int y = g2.min.y;

                //! \bug https://bugreports.qt.io/browse/QTBUG-65496
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

                for (const auto& text : p.text)
                {
                    const QString qtext = QString::fromUtf8(text.c_str());
                    const QSize size = fm.size(Qt::TextSingleLine, qtext);
                    const math::Box2i g3(
                        x, y, size.width() + margin * 2,
                        size.height() + margin * 2);
                    painter.fillRect(
                        g3.min.x, g3.min.y, g3.w(), g3.h(),
                        QColor(0, 0, 0, 127));
                    painter.drawText(
                        g3.min.x + margin, g3.min.y + margin + fm.ascent(),
                        qtext);
                }
            }
        }

        void Viewport::_textUpdate()
        {
            TLRENDER_P();
            p.text.clear();
            p.text.push_back(string::Format("FPS: {0} ({1} dropped)")
                                 .arg(p.fps, 2, 4)
                                 .arg(p.droppedFrames));
            update();
        }
    } // namespace play_qt
} // namespace tl
