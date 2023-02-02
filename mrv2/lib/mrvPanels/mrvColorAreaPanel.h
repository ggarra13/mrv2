// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvPanelWidget.h"

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    namespace area
    {
        class Info;
    }

    class ColorAreaPanel : public PanelWidget
    {
    public:
        ColorAreaPanel( ViewerUI* ui );
        ~ColorAreaPanel();

        void add_controls() override;


        void update( const area::Info& info );


    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
