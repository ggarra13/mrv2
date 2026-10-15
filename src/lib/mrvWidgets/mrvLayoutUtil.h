// mrvWidgets/mrvLayoutUtil.h
#pragma once
#include <FL/Fl_Widget.H>

namespace mrv
{
    //! Walk up the widget tree from w, calling layout() on every
    //! ancestor that knows how to reflow its own children (Pack,
    //! AnnotationGroup, PanelGroup...), then redraw the top window.
    //!
    //! Needed because our Pack/PanelGroup keep layout() (recompute
    //! sizes/positions) separate from redraw() (repaint only) --
    //! stock Fl_Pack folds both into draw(), ours doesn't, so a
    //! resized child must explicitly ask its ancestors to re-layout.
    void relayout(Fl_Widget* w);
}
