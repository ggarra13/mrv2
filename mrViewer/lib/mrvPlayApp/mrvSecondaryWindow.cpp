// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "mrvPlayApp/mrvSecondaryWindow.h"

#include "mrvPlayApp/App.h"
#include "mrvPlayApp/mrvSettingsObject.h"

#include "mrvGL/mrvGLViewport.h"

#include "mrvWidgets/mrvMainWindow.h"


namespace mrv
{
    struct SecondaryWindow::Private
    {
        App* app = nullptr;
        MainWindow* mainWindow = nullptr;
        GLViewport* viewport = nullptr;
    };

    SecondaryWindow::SecondaryWindow( App* app ) :
        _p(new  Private)
    {
        TLRENDER_P();

        p.app = app;

        int X = 0, Y = 0, W = 1280, H = 720;

        p.mainWindow = new MainWindow( W, H, "Secondary" );
        
        p.viewport = new GLViewport( 0, 0, W, H );
        p.viewport->setContext( app->getContext() );

        p.mainWindow->resizable( p.viewport );

        p.mainWindow->end();

        SettingsObject* settings = app->settingsObject();
        std::string key = "gui/Secondary/WindowVisible";
        std_any value = settings->value( key );
        int visible = value.empty() ? 0 : std_any_cast<int>( value );

        key = "gui/Secondary/WindowX";
        value = settings->value( key );
        X = value.empty() ? X : std_any_cast<int>( value );
            
        key = "gui/Secondary/WindowY";
        value = settings->value( key );
        Y = value.empty() ? Y : std_any_cast<int>( value );
            
        key = "gui/Secondary/WindowW";
        value = settings->value( key );
        W = value.empty() ? W : std_any_cast<int>( value );
            
        key = "gui/Secondary/WindowH";
        value = settings->value( key );
        H = value.empty() ? H : std_any_cast<int>( value );
            
        p.mainWindow->resize( X, Y, W, H );
            
        if ( visible )
        {
            p.mainWindow->show();
        }
        else
        {
            p.mainWindow->hide();
        }
    }

    SecondaryWindow::~SecondaryWindow()
    {
        TLRENDER_P();
        
        SettingsObject* settings = p.app->settingsObject();
        
        std::string key = "gui/Secondary/WindowVisible";
        MainWindow* w = p.mainWindow;
        int visible = w->visible();
        settings->setValue( key, visible );
        
        if ( visible )
        {
            key = "gui/Secondary/WindowX";
            settings->setValue( key, w->x() );
            
            key = "gui/Secondary/WindowY";
            settings->setValue( key, w->y() );
            
            key = "gui/Secondary/WindowW";
            settings->setValue( key, w->w() );
            
            key = "gui/Secondary/WindowH";
            settings->setValue( key, w->h() );
        }

    }

    GLViewport* SecondaryWindow::viewport() const
    {
        return _p->viewport;
    }

}
