
#include <FL/Fl_Menu_Item.H>

#include "mrvHDRView.h"

namespace mrv
{
    void select_ndi_source_cb(Fl_Menu_* m, HDRUI* ui)
    {
        const Fl_Menu_Item* item = m->mvalue();
        if (!item || !item->label())
            return;

        const std::string& source = item->label();
        ui->uiView->setNDISource(source);
    }
}
