// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvToolWidget.h"

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    namespace area
    {
        class Info;
    }
    
    class VectorscopeTool : public ToolWidget
    {
    public:
        VectorscopeTool( ViewerUI* ui );
        ~VectorscopeTool();

        void add_controls() override;


        void update( const area::Info& info );

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
