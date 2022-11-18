#pragma once

#include <vector>

#include <FL/Fl_RGB_Image.H>

#include <tlCore/Time.h>

#include "mrvWidgets/mrvClipButton.h"
#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class HorSlider;
    
    class CompareTool : public ToolWidget
    {
    public:
      HorSlider* wipeX;
      HorSlider* wipeY;
      HorSlider* wipeRotation;
      HorSlider* overlay;
    public:
        CompareTool( ViewerUI* ui );
        ~CompareTool();

        void clear_controls();
        void add_controls() override;
        
        void redraw();
        
        void refresh();
        void compareThumbnail( const int64_t id,
                               const std::vector< std::pair<otime::RationalTime,
                               Fl_RGB_Image*> >& thumbnails,
                               ClipButton* w);

    protected:
        void cancel_thumbnails();
        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
