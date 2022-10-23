#pragma once

#include <vector>

#include <FL/Fl_Box.H>
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
        
        void add_controls() override;

        void refresh();
        void createdThumbnail( const int64_t id,
                               const std::vector< std::pair<otime::RationalTime,
                               Fl_RGB_Image*> >& thumbnails,
                               Fl_Box* w);
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
