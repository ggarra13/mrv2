// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


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
        ui->uiView->fill_menu(m);
    }
    
    void apply_metadata_cb(Fl_Menu_* m, HDRUI* ui)
    {
        ui->uiView->toggle_hdr_metadata();
        ui->uiView->fill_menu(m);
    }
    
    void toggle_fullscreen_cb(Fl_Menu_* m, HDRUI* ui)
    {
        ui->uiView->toggle_fullscreen();
        ui->uiView->fill_menu(m);
    }
}
