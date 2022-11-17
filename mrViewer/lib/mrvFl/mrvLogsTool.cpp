
#include <string>
#include <vector>
#include <map>

#include <tlCore/StringFormat.h>


#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvToolsCallbacks.h"
#include "mrvFunctional.h"


#include "mrViewer.h"


namespace mrv
{

    struct LogsTool::Private
    {
        App*                               app;
        LogDisplay*                 listWidget;
        Fl_Button*                  clearButton;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
    };

    
    LogsTool::LogsTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( "Logs" );
    
        // @todo: add an icon for logs
        // Fl_SVG_Image* svg = load_svg( "Logs.svg" );
        // g->image( svg );
    
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete logsTool; logsTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    LogsTool::~LogsTool()
    {
    }


    void LogsTool::dock()
    {
        ToolWidget::dock();
        // @todo: avoid scrolling issues
    }
    
    void LogsTool::undock()
    {
        ToolWidget::undock();
        ToolWindow* w = g->get_window();
        // Resize window to a good size
        w->resize( 40, 40, 512, 512 );
    }

    void LogsTool::add_controls()
    {
        TLRENDER_P();
        
        g->clear();
    
        g->begin();

        _r->listWidget = new LogDisplay( g->x(), g->y()+20, g->w(),
                                         p.ui->uiViewGroup->h() - 50 );
    
        _r->clearButton = new Fl_Button( g->x(), g->y() + _r->listWidget->h(),
                                         30, 30 );
        _r->clearButton->image( load_svg("Clear.svg") );
        _r->clearButton->tooltip( _("Clear the messages") );
        _r->clearButton->callback( []( Fl_Widget* w, void* d )
            {
                LogDisplay* log = static_cast< LogDisplay* >( d );
                log->clear();
            }, _r->listWidget );
            
        g->end();

        _r->logObserver = observer::ListObserver<log::Item>::create(
            p.ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Message:
                        {
                            const std::string& msg = string::Format("{0} {1}: {2}\n").
                                                     arg(i.time).
                                                     arg(i.prefix).
                                                     arg(i.message);
                            _r->listWidget->info( msg.c_str() );
                            break;
                        }
                        case log::Type::Warning:
                        {
                            const std::string& msg = string::Format("{0} Warning {1}: {2}\n").
                                                     arg(i.time).
                                                     arg(i.prefix).
                                                     arg(i.message);
                            _r->listWidget->warning( msg.c_str() );
                            break;
                        }
                        case log::Type::Error:
                        {
                            const std::string& msg = string::Format("{0} ERROR {1}: {2}\n").
                                                     arg(i.time).
                                                     arg(i.prefix).
                                                     arg(i.message);
                            _r->listWidget->error( msg.c_str() );
                            if ( LogDisplay::prefs == LogDisplay::kWindowOnError )
                            {
                                if ( !logsTool ) logs_tool_grp( NULL, _p->ui  );
                                logsTool->undock();
                            }
                            else if ( LogDisplay::prefs == LogDisplay::kDockOnError )
                            {
                                if ( !logsTool ) logs_tool_grp( NULL, _p->ui  );
                                logsTool->dock();
                            }
                            break;
                        }
                        }
                    }
                });
        
    }
    

}
