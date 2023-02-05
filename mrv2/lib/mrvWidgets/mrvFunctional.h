// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Widget.H>
#include <functional>


template<typename W, typename = typename std::enable_if<std::is_base_of<Fl_Widget, W>::value>::type>
class Widget: public W {
    std::function<void(W *)> fn_ = nullptr;
public:
    Widget(int x, int y, int w, int h, const char *l = 0): W(x, y, w, h, l) {}
    void callback(const std::function<void(W *)> &fn) {
        fn_ = fn;
        auto adapter = [](Fl_Widget* w, void* data) 
            {
                auto f = (std::function<void(W *)> *)data;
                (*f)((W *)w);
            };
        W::callback(adapter, (void *)&fn_);
    }
};
