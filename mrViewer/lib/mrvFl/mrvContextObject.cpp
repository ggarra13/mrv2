// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <mrvFl/mrvContextObject.h>

#include <tlCore/Context.h>

#include <FL/Fl.H>

namespace mrv
{
    using namespace tl;

    struct ContextObject::Private
    {
        std::shared_ptr<system::Context> context;
    };

    ContextObject::ContextObject(
        const std::shared_ptr<system::Context>& context ) :
        _p(new Private)
    {
        _p->context = context;

        Fl::add_timeout( 0.005, (Fl_Timeout_Handler) timerEvent_cb,
                         this );
    }

    ContextObject::~ContextObject()
    {}

    const std::shared_ptr<system::Context>& ContextObject::context() const
    {
        return _p->context;
    }

    void ContextObject::timerEvent()
    {
        _p->context->tick();
        Fl::repeat_timeout( 0.005, (Fl_Timeout_Handler) timerEvent_cb,
                            this );
    }

    void ContextObject::timerEvent_cb( void* d )
    {
        ContextObject* t = static_cast< ContextObject* >( d );
        t->timerEvent();
    }
}
