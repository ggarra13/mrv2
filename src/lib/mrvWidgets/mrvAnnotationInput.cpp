// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvAnnotationInput.h"

#include "mrViewer.h"

namespace mrv
{

    void AnnotationInput::draw()
    {
        Fl_Multiline_Input::draw();

        if (locked_)
            return;

        // Two pixel border while editing.
        fl_color(ann_colors::border());
        fl_rect(x(), y(), w(), h());
        fl_rect(x() + 1, y() + 1, w() - 2, h() - 2);
    }

    int AnnotationInput::handle(int e)
    {
        if (locked_)
            return 0;

        int ret = Fl_Multiline_Input::handle(e);

        if (!value())
            return ret;

        MyViewport* view = App::ui->uiView;
        if (!view)
            return ret;
        auto player = view->getTimelinePlayer();
        if (!player)
            return ret;
        auto annotation = player->getAnnotation();
        if (!annotation)
        {
            annotation = player->createAnnotation(false);
            if (!annotation)
                return ret;
        }

        std::shared_ptr< draw::Shape > s;
        for (const auto& shape : annotation->shapes)
        {
            if (dynamic_cast< draw::NoteShape* >(shape.get()))
            {
                s = shape;
                break;
            }
        }
        if (!s)
        {
            s = std::make_shared< draw::NoteShape >();
            annotation->push_back(s);
        }
        auto shape = dynamic_cast< draw::NoteShape* >(s.get());
        if (!shape)
            return ret;
        shape->text = value();

        return ret;
    }

} // namespace mrv
