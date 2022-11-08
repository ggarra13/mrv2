#include "mrvCore/mrvSequence.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "mrvFl/mrvFileRequester.h"
#include "mrvWidgets/mrvToolGroup.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvCallbacks.h"

#include "mrvPlayApp/mrvFilesModel.h"
#include "mrvPlayApp/App.h"

#include "make_ocio_chooser.h"
#include "mrvHotkeyUI.h"
#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace {
  const char* kModule = "cback";
}

namespace mrv
{
  
  static void refresh_tool_grp()
  {
    if ( filesTool )    filesTool->refresh();
    if ( compareTool ) compareTool->refresh();
  }

  static void reset_timeline( ViewerUI* ui )
  {
    ui->uiTimeline->setTimelinePlayer( nullptr );
    otio::RationalTime start = otio::RationalTime( 1, 24 );
    otio::RationalTime end   = otio::RationalTime( 50, 24 );
    ui->uiFrame->setTime( start );
    ui->uiStartFrame->setTime( start );
    ui->uiEndFrame->setTime( end );
  }
    
  static void printIndices( const std::vector< int >& Bindexes )
  {
    std::cerr << "Indices now: " << std::endl;
    for ( auto& i : Bindexes )
      {
	std::cerr << i << " ";
      }
    std::cerr << std::endl;
  }

  void open_files_cb( const std::vector< std::string >& files, ViewerUI* ui )
  {
    for ( const auto& file : files )
      {
	ui->app->open( file );
      }
    ui->uiMain->fill_menu( ui->uiMenuBar );
    refresh_tool_grp();
  }

  void open_cb( Fl_Widget* w, ViewerUI* ui )
  {
    const stringArray& files = open_image_file( NULL, true, ui );
    open_files_cb( files, ui );
  }

  void open_recent_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    if ( !item || !item->label() ) return;
    stringArray files;
    std::string file = item->label();
    files.push_back( file );
    open_files_cb( files, ui );
  }
  
  void open_separate_audio_cb( Fl_Widget* w, ViewerUI* ui )
  {
    ui->app->openSeparateAudioDialog();
    ui->uiMain->fill_menu( ui->uiMenuBar );
    refresh_tool_grp();
  }

  void open_directory_cb( Fl_Widget* w, ViewerUI* ui )
  {
    std::string dir = open_directory(NULL, ui);
    if (dir.empty()) return;

    stringArray movies, sequences, audios;
    parse_directory( dir, movies, sequences, audios );

    for ( const auto& movie : movies )
      {
	ui->app->open( movie );
      }
    for ( const auto& sequence : sequences )
      {
	ui->app->open( sequence );
      }
    for ( const auto& audio : audios )
      {
	ui->app->open( audio );
      }

    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void previous_file_cb( Fl_Widget* w, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto Aindex = model->observeAIndex()->get();
    if ( Aindex > 0 ) Aindex -= 1;
    model->setA( Aindex );
  }
    
  void next_file_cb( Fl_Widget* w, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto Aindex = model->observeAIndex()->get();
    auto images = model->observeFiles()->get();
    int num = (int) images.size() - 1;
    if ( Aindex < num ) Aindex += 1;
    model->setA( Aindex );
  }

    

  void close_current_cb( Fl_Widget* w, ViewerUI* ui )
  {
    auto model = ui->app->filesModel(); 
    model->close();
    
    ui->uiMain->fill_menu( ui->uiMenuBar );
    
    auto images = model->observeFiles()->get();
    if ( images.empty() ) reset_timeline( ui );
    
    refresh_tool_grp();
  }

  void close_all_cb( Fl_Widget* w, ViewerUI* ui )
  { 
    auto model = ui->app->filesModel(); 
    model->closeAll();
    
    ui->uiMain->fill_menu( ui->uiMenuBar );
    
    reset_timeline( ui );
    
    refresh_tool_grp();
  }

  void exit_cb( Fl_Widget* w, ViewerUI* ui )
  {
    // Store window preferences
    if ( colorTool )    colorTool->save();
    if ( filesTool )     filesTool->save();
    if ( compareTool )  compareTool->save();
    if ( settingsTool ) settingsTool->save();
        
    // Save preferences
    Preferences::save();
        
    // Delete all windows which will close all threads.
    delete ui->uiInfo; ui->uiInfo = nullptr;
    delete ui->uiMain; ui->uiMain = nullptr;
    delete ui->uiPrefs; ui->uiPrefs = nullptr;
    delete ui->uiHotkey; ui->uiHotkey = nullptr;

    ToolGroup::hide_all();
    // The program should exit cleanly from the Fl::run loop now
  }


  void minify_nearest_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
    o.imageFilters.minify = timeline::ImageFilter::Nearest;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  void minify_linear_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
    o.imageFilters.minify = timeline::ImageFilter::Linear;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  void magnify_nearest_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
    o.imageFilters.magnify = timeline::ImageFilter::Nearest;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  void magnify_linear_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
    o.imageFilters.magnify = timeline::ImageFilter::Linear;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  void mirror_x_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    timeline::DisplayOptions& d = ui->uiView->getDisplayOptions(-1);
    d.mirror.x ^= 1;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  void mirror_y_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    timeline::DisplayOptions& d = ui->uiView->getDisplayOptions(-1);
    d.mirror.y ^= 1;
    ui->uiMain->fill_menu( ui->uiMenuBar );
    ui->uiView->redraw();
  }

  static void toggle_channel( Fl_Menu_Item* item,
			      TimelineViewport* view,
			      const timeline::Channels channel )
  {
    const timeline::DisplayOptions& d = view->getDisplayOptions(-1);
    if ( d.channels == channel ) item->uncheck();

    view->toggleDisplayChannel( channel );
  }

  void toggle_red_channel_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    const timeline::Channels channel = timeline::Channels::Red;
    toggle_channel( item, ui->uiView, channel );
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void toggle_green_channel_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    const timeline::Channels channel = timeline::Channels::Green;
    toggle_channel( item, ui->uiView, channel );
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void toggle_blue_channel_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    const timeline::Channels channel = timeline::Channels::Blue;
    toggle_channel( item, ui->uiView, channel );
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void toggle_alpha_channel_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    const timeline::Channels channel = timeline::Channels::Alpha;
    toggle_channel( item, ui->uiView, channel );
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void change_media_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    Fl_Menu_Item* item = nullptr;
    Fl_Menu_Item* picked = const_cast< Fl_Menu_Item* >( m->mvalue() );

    int start = m->find_index(_("Compare/Current"));

    // Find submenu's index
    int idx = m->find_index(picked) - start - 1;

    auto model = ui->app->filesModel();


    item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/A") ) );
    if ( item->checked() )
      {
	model->setA( idx );
	return;
      }
    item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/B") ) );
    if ( item->checked() )
      {
	auto Bindexes = model->observeBIndexes()->get();
	if ( !picked->checked() )
	  {
	    model->setB( -1, true );
	    Bindexes = model->observeBIndexes()->get();
	    //printIndices( Bindexes );
	  }
	else
	  {
	    // Add index to B indexes list
	    model->setB( idx, true );
	    Bindexes = model->observeBIndexes()->get();
	    //printIndices( Bindexes );
	  }
	return;
      }
  }

  void compare_wipe_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Wipe;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
    ui->uiView->redraw();
  }

  void A_media_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto images = model->observeFiles()->get();
    if ( images.empty() ) return;

    auto Aindex = model->observeAIndex()->get();

    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::A;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );


    size_t start = m->find_index(_("Compare/Current")) + 1;

    // Find submenu's index
    size_t num = images.size() + start;
    for ( size_t i = start; i < num; ++i )
      {
	Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
	const char* label = item->label();
	size_t idx = i-start;
	if ( idx == Aindex )
	  {
	    item->set();
	  }
	else
	  item->clear();
      }
  }

  void B_media_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto images = model->observeFiles()->get();
    if ( images.empty() ) return;

    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::B;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );


    size_t start = m->find_index(_("Compare/Current")) + 1;

    // Find submenu's index
    size_t num = images.size() + start;
    auto Bindexes = model->observeBIndexes()->get();


    for ( size_t i = start; i < num; ++i )
      {
	Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
	const char* label = item->label();
	size_t idx = i-start;
	if ( !Bindexes.empty() && idx == Bindexes[0] )
	  {
	    item->set();
	  }
	else
	  item->clear();
      }

  }

  void compare_overlay_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Overlay;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
  }

  void compare_difference_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Difference;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
  }

  void compare_horizontal_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Horizontal;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
  }

  void compare_vertical_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Vertical;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
  }

  void compare_tile_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    auto model = ui->app->filesModel();
    auto compare = model->observeCompareOptions()->get();
    compare.mode = timeline::CompareMode::Tile;
    model->setCompareOptions( compare );
    ui->uiView->setCompareOptions( compare );
  }

  void window_cb( Fl_Menu_* m, ViewerUI* ui )
  {

    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );

    std::string tmp = item->text;
    Fl_Window* w = nullptr;

    if ( tmp == _("Files") )
      {
	files_tool_grp( m, ui );
	return;
      }
    else if ( tmp == _("Media Information") )
      w = ui->uiInfo->uiMain;
    else if ( tmp == _("Color Information") )
      w = nullptr;
    else if ( tmp == _("Color Controls") )
      {
	color_tool_grp( m, ui );
	return;
      }
    else if ( tmp == _("Compare") )
      {
	compare_tool_grp( m, ui );
	return;
      }
    else if ( tmp == _("Settings") )
      {
	settings_tool_grp( m, ui );
	return;
      }
    else if ( tmp == _("Action Tools") )
      w = nullptr;
    else if ( tmp == _("Preferences") )
      w = ui->uiPrefs->uiMain;
    else if ( tmp == _("Histogram") )
      w = nullptr;
    else if ( tmp == _("Vectorscope") )
      w = nullptr;
    else if ( tmp == _("Waveform") )
      w = nullptr;
    else if ( tmp == _("Connections") )
      w = nullptr;
    else if ( tmp == _("Hotkeys") )
      w = ui->uiHotkey->uiMain;
    else if ( tmp == _("Logs") )
      w = nullptr;
    else if ( tmp == _("About") )
      w = ui->uiAbout->uiMain;
    else
      {
	LOG_ERROR( _("Unknown window ") << tmp );
	return; // Unknown window
      }
        
    if ( w ) {
      w->show();
      if ( w == ui->uiInfo->uiMain )
	ui->uiInfo->uiInfoText->refresh();
    }
  }

  bool has_tools_grp = true,
    has_menu_bar = true,
    has_top_bar = true,
    has_bottom_bar = true,
      has_pixel_bar = true,
      has_dock_grp = true,
      has_media_info_window = true,
      has_preferences_window = true,
      has_hotkeys_window = true;

  void save_ui_state( ViewerUI* ui, Fl_Group* bar )
  {
    if ( bar == ui->uiMenuGroup )
      has_menu_bar   = ui->uiMenuGroup->visible();
    else if ( bar == ui->uiTopBar )
      has_top_bar    = ui->uiTopBar->visible();
    else if ( bar == ui->uiBottomBar )
      has_bottom_bar = ui->uiBottomBar->visible();
    else if ( bar == ui->uiPixelBar )
      has_pixel_bar  = ui->uiPixelBar->visible();
    else if ( bar == ui->uiToolsGroup )
      has_tools_grp  = ui->uiToolsGroup->visible();
    else if ( bar == ui->uiDockGroup )
      has_dock_grp  = ui->uiDockGroup->visible();

  }
  void save_ui_state( ViewerUI* ui )
  {
    has_menu_bar   = ui->uiMenuGroup->visible();
    has_top_bar    = ui->uiTopBar->visible();
    has_bottom_bar = ui->uiBottomBar->visible();
    has_pixel_bar  = ui->uiPixelBar->visible();
    has_tools_grp  = ui->uiToolsGroup->visible();
    has_dock_grp = ui->uiDockGroup->visible();

    //@todo: add floating windows too
    has_media_info_window = ui->uiInfo->uiMain->visible();
    has_preferences_window = ui->uiPrefs->uiMain->visible();
    has_hotkeys_window = ui->uiHotkey->uiMain->visible();
  }


  void hide_ui_state( ViewerUI* ui )
  {

    int W = ui->uiMain->w();
    int H = ui->uiMain->h();

    if ( has_tools_grp )
      {
	ui->uiToolsGroup->hide();
      }

    if ( has_bottom_bar ) {
      ui->uiBottomBar->hide();
    }
    if ( has_pixel_bar ) {
      ui->uiPixelBar->hide();
    }
    if ( has_top_bar ) {
      ui->uiTopBar->hide();
    }
    if ( has_menu_bar )
    {
	ui->uiMenuGroup->hide();
    }
    if ( has_dock_grp )
    {
        ui->uiDockGroup->hide();
    }
    
    if ( has_media_info_window )  ui->uiInfo->uiMain->hide();
    if ( has_preferences_window ) ui->uiPrefs->uiMain->hide();
    if ( has_hotkeys_window )     ui->uiHotkey->uiMain->hide();

    ToolGroup::hide_all();
    
    ui->uiRegion->layout();
  }

  void toggle_action_tool_bar( Fl_Menu_* m, ViewerUI* ui )
  {
    Fl_Group* bar = ui->uiToolsGroup;

    if ( bar->visible() )
      bar->hide();
    else
      bar->show();
        
    ui->uiViewGroup->layout();
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size )
  {
    if ( bar->visible() )
      {
	bar->hide();
      }
    else
      {
	bar->show();
      }

    ui->uiRegion->layout();
  }


  void restore_ui_state( ViewerUI* ui )
  {

    if ( has_menu_bar )
      {
	if ( !ui->uiMenuGroup->visible() )    {
	  ui->uiMain->fill_menu( ui->uiMenuBar );
	  ui->uiMenuGroup->show();
	}
      }

    if ( has_top_bar )
      {
	if ( !ui->uiTopBar->visible() )    {
	  ui->uiTopBar->show();
	}
      }

    if ( has_bottom_bar )
      {
	if ( !ui->uiBottomBar->visible() )  {
	  ui->uiBottomBar->show();
	}
      }

    if ( has_pixel_bar )
      {
	if ( !ui->uiPixelBar->visible() )  {
	  ui->uiPixelBar->show();
	}
      }


    ui->uiViewGroup->layout();

    if ( has_tools_grp )
      {
	if ( !ui->uiToolsGroup->visible() )
	  {
	    ui->uiToolsGroup->show();
	  }
      }

    if ( has_dock_grp )
    {
	if ( !ui->uiDockGroup->visible() )
	  {
	    ui->uiDockGroup->show();
	  }
    }

    ui->uiRegion->layout();

    //@todo: add showing of floating windows too
    
    if ( has_media_info_window )  ui->uiInfo->uiMain->show();
    if ( has_preferences_window ) ui->uiPrefs->uiMain->show();
    if ( has_hotkeys_window )     ui->uiHotkey->uiMain->show();

    ToolGroup::show_all();
  }

  void hud_toggle_cb( Fl_Menu_* o, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( o->mvalue() );
    GLViewport* view = ui->uiView;
    view->setHudActive( item->checked() );
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  void masking_cb( Fl_Menu_* w, ViewerUI* ui )
  {
    Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
    float mask = atof( item->label() );
    ui->uiView->setMask( mask );
  }

  void hud_cb( Fl_Menu_* o, ViewerUI* ui )
  {
    const Fl_Menu_Item* item = o->mvalue();

    int i;
    Fl_Group* menu = ui->uiPrefs->uiPrefsHud;
    int num = menu->children();
    for ( i = 0; i < num; ++i )
      {
	const char* fmt = menu->child(i)->label();
	if (!fmt) continue;
	if ( strcmp( fmt, item->label() ) == 0 ) break;
      }

    GLViewport* view = ui->uiView;
    unsigned int hud = view->getHudDisplay();
    hud ^= ( 1 << i );
    view->setHudDisplay( (HudDisplay) hud );
  }

  // Playback callbacks
  void play_forwards_cb( Fl_Menu_*, ViewerUI* ui )
  {
    ui->uiView->playForwards();
  }

  void play_backwards_cb( Fl_Menu_*, ViewerUI* ui )
  {
    ui->uiView->playBackwards();
  }

  void stop_cb( Fl_Menu_*, ViewerUI* ui )
  {
    ui->uiView->stop();
  }

  void toggle_playback_cb( Fl_Menu_* m, ViewerUI* ui )
  {
    ui->uiView->togglePlayback();
    ui->uiMain->fill_menu( ui->uiMenuBar );
  }

  static void playback_loop_mode( ViewerUI* ui, timeline::Loop mode )
  {
    ui->uiLoopMode->value( (int)mode );
    ui->uiLoopMode->do_callback();
  }

  void playback_loop_cb( Fl_Menu_*, ViewerUI* ui )
  {
    playback_loop_mode( ui, timeline::Loop::Loop );
  }
  void playback_once_cb( Fl_Menu_*, ViewerUI* ui )
  {
    playback_loop_mode( ui, timeline::Loop::Once );
  }
  void playback_ping_pong_cb( Fl_Menu_*, ViewerUI* ui )
  {
    playback_loop_mode( ui, timeline::Loop::PingPong );
  }
    
  // OCIO callbacks
  void attach_ocio_ics_cb( Fl_Menu_*, ViewerUI* ui )
  {
    mrv::PopupMenu* w = ui->uiICS;
    std::string ret = make_ocio_chooser( w->label(),
					 OCIOBrowser::kInputColorSpace );
    if ( ret.empty() ) return;

    for ( size_t i = 0; i < w->children(); ++i )
      {
	const Fl_Menu_Item* o = w->child(i);
	if ( !o || !o->label() ) continue;

	if ( ret == o->label() )
	  {
	    w->copy_label( o->label() );
	    w->value(i);
	    w->do_callback();
	    ui->uiView->redraw();
	    break;
	  }
      }
  }
    
  void attach_ocio_display_cb( Fl_Menu_*, ViewerUI* ui )
  {
    std::string ret = make_ocio_chooser( Preferences::OCIO_Display,
					 OCIOBrowser::kDisplay );
    if ( ret.empty() ) return;
    Preferences::OCIO_Display = ret;
    ui->uiView->redraw();
  }
    
  void attach_ocio_view_cb( Fl_Menu_*, ViewerUI* ui )
  {
    std::string ret = make_ocio_chooser( Preferences::OCIO_View,
					 OCIOBrowser::kView );
    if ( ret.empty() ) return;
    Preferences::OCIO_View = ret;
    Fl_Menu_Button* m = ui->gammaDefaults;
    for ( int i = 0; i < m->size(); ++i )
      {
	const char* lbl = m->menu()[i].label();
	if ( !lbl ) continue;
	if ( ret == lbl )
	  {
	    m->value(i);
	    break;
	  }
      }
    m->copy_label( _(ret.c_str()) );
    m->redraw();
    ui->uiView->redraw();
  }

  void video_levels_from_file_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.videoLevels = timeline::InputVideoLevels::FromFile;
    ui->uiView->redraw();
  }
    
  void video_levels_legal_range_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.videoLevels = timeline::InputVideoLevels::LegalRange;
    ui->uiView->redraw();
  }
    
  void video_levels_full_range_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.videoLevels = timeline::InputVideoLevels::FullRange;
    ui->uiView->redraw();
  }
    
  void alpha_blend_none_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.alphaBlend = timeline::AlphaBlend::None;
    ui->uiView->redraw();
  }
    
  void alpha_blend_straight_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.alphaBlend = timeline::AlphaBlend::Straight;
    ui->uiView->redraw();
  }
    
  void alpha_blend_premultiplied_cb( Fl_Menu_*, ViewerUI* ui )
  {
    timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
    o.alphaBlend = timeline::AlphaBlend::Premultiplied;
    ui->uiView->redraw();
  }
    
}
