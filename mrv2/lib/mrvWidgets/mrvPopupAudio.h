// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvPopupMenu.h"

namespace mrv {

class PopupAudio : public PopupMenu
{
public:
    PopupAudio(int,int,int,int,const char* =0);
    virtual ~PopupAudio() {};
    virtual int handle(int e) override;
};

}
