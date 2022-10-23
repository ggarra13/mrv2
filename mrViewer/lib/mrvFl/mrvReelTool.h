#pragma once

#include <vector>

#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>

#include <tlCore/Time.h>

#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;
    
    class ReelTool : public ToolWidget
    {
    public:
        ReelTool( ViewerUI* ui );
        ~ReelTool();

        void clear_controls();
        void add_controls() override;

        
        void redraw();
        
        void refresh();
        void reelThumbnail( const int64_t id,
                            const std::vector< std::pair<otime::RationalTime,
                            Fl_RGB_Image*> >& thumbnails,  Fl_Button* w);
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
