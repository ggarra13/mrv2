// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvGLTextEdit.h"

#include "mrvGL/mrvGLShape.h"

#include "mrViewer.h"

namespace mrv
{
    void updateText_cb(Fl_Widget* w, TextEdit* c)
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
#ifdef USE_OPENGL2
                    {
                        auto s = dynamic_cast<GL2TextShape*>(shape.get());
                        if (s)
                            c->textOutput.value(s->text.c_str());
                    }
#endif
                    {
                        auto s = dynamic_cast<GLTextShape*>(shape.get());
                        if (s)
                            c->textOutput.value(s->text.c_str());
                    }
                    c->textOutput.redraw();
                    return;
                }
            }
        }
    }

    void editText_cb(Fl_Widget* w, TextEdit* c)
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

        int idx = 0;
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
                    view->editText(shape, idx);
                    c->hide();
                    return;
                }
            }
            ++idx;
        }
    }

    TextEdit::TextEdit(int W, int H, const char* L) :
        Fl_Double_Window(W, H, L),
        textOutput(60, 10, W - 70, 100),
        textAnnotations(60, 110, W - 70, 30, _("Text")),
        editButton(60, 150, W - 70, 30, _("Edit Text"))
    {
        end();
        textAnnotations.callback((Fl_Callback*)updateText_cb, this);
        editButton.callback((Fl_Callback*)editText_cb, this);
    }

    void TextEdit::add(const std::string& text)
    {
        size_t pos = text.find('\n');
        if (pos == std::string::npos)
            pos = 40;

        std::string line = text.substr(0, pos);
        if (text.size() > pos)
            line += " ...";
        textAnnotations.add(line.c_str());
        textAnnotations.value(0);
        textAnnotations.do_callback();
    }

} // namespace mrv
