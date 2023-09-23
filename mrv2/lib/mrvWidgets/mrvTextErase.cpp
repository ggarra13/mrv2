// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvTextErase.h"

#include "mrvGL/mrvGLShape.h"

#include "mrViewer.h"

namespace mrv
{
    void deleteText_cb(Fl_Widget* w, TextErase* c)
    {
        ViewerUI* ui = App::ui;

        Viewport* view = ui->uiView;

        auto player = view->getTimelinePlayer();
        if (!player)
            return;

        int selected = c->textAnnotations.value();
        int count = -1;
        auto annotation = player->getAnnotation();
        if (!annotation)
            return;

        for (const auto& shape : annotation->shapes)
        {
            if (
#ifdef USE_OPENGL2
                dynamic_cast<GL2TextShape*>(shape.get()) ||
#endif
                dynamic_cast<GLTextShape*>(shape.get()))
            {
                ++count;
                if (count == selected)
                {
                    annotation->remove(shape);
                    c->textAnnotations.remove(selected);
                    c->textAnnotations.value(0);
                    c->textAnnotations.redraw();
                    view->redrawWindows();
                    if (c->textAnnotations.size() <= 1)
                    {
                        c->hide();
                    }
                    return;
                }
            }
        }
    }

    TextErase::TextErase(int W, int H, const char* L) :
        Fl_Double_Window(W, H, L),
        textAnnotations(60, 10, W - 70, 30, _("Text")),
        deleteButton(60, 50, W - 70, 30, _("Delete Text"))
    {
        end();
        deleteButton.callback((Fl_Callback*)deleteText_cb, this);
    }

    void TextErase::add(const std::string& text)
    {
        textAnnotations.add(text.c_str());
        textAnnotations.value(0);
    }

} // namespace mrv
