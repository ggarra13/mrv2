
#include <string>
#include <vector>
#include <map>

#include <tlCore/StringFormat.h>


#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvToolsCallbacks.h"
#include "mrvFunctional.h"


#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace {
    const char* kModule = "logs";
}


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
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete logsTool; logsTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    LogsTool::~LogsTool()
    {
    }



    void LogsTool::add_controls()
    {
        TLRENDER_P();
	
        
        g->clear();
    
        g->begin();

        _r->listWidget = new LogDisplay( g->x(), g->y()+20, g->w(),
                                         p.ui->uiViewGroup->h() - 50 );
    
        _r->clearButton = new Fl_Button( g->x(), g->y() + _r->listWidget->h(),
                                         g->w(), 30 );
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
                        const std::string& msg = string::Format("{0} {1}: {2}").
                                                 arg(i.time).
                                                 arg(i.prefix).
                                                 arg(i.message);
                        switch (i.type)
                        {
                        case log::Type::Message:
                            _r->listWidget->info( msg.c_str() );
                            break;
                        case log::Type::Warning:
                            _r->listWidget->warning( msg.c_str() );
                            break;
                        case log::Type::Error:
                            _r->listWidget->error( msg.c_str() );
                            break;
                        }
                    }
                });
        
    }
    

}
