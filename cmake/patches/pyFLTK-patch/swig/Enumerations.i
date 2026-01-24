/* File : Enumerations.i */
//%module Enumerations

// hack to convince SWIG that Fl_Color is something different than it really is!
%apply unsigned int { Fl_Color};
%apply const unsigned int& { const Fl_Color&};
// end hack


%{


#include "FL/Enumerations.H"
%}

%include "FL/Enumerations.H"
int fl_command_modifier();
int fl_control_modifier();

%pythoncode %{
# events
FL_KEYBOARD=FL_KEYDOWN

# color defines
FL_FOREGROUND_COLOR  = 0
FL_BACKGROUND2_COLOR = 7
FL_INACTIVE_COLOR    = 8
FL_SELECTION_COLOR   = 15

FL_GRAY0   = 32
FL_DARK3   = 39
FL_DARK2   = 45
FL_DARK1   = 47
FL_BACKGROUND_COLOR  = 49
FL_LIGHT1  = 50
FL_LIGHT2  = 52
FL_LIGHT3  = 54

FL_BLACK   = 56
FL_RED     = 88
FL_GREEN   = 63
FL_YELLOW  = 95
FL_BLUE    = 216
FL_MAGENTA = 248
FL_CYAN    = 223
FL_DARK_RED = 72

FL_DARK_GREEN    = 60
FL_DARK_YELLOW   = 76
FL_DARK_BLUE     = 136
FL_DARK_MAGENTA  = 152
FL_DARK_CYAN     = 140

FL_WHITE         = 255

FL_FREE_COLOR=16           
FL_NUM_FREE_COLOR=16       
FL_GRAY_RAMP=32           
FL_NUM_GRAY=24                     
FL_GRAY=49
FL_COLOR_CUBE=56           
FL_NUM_RED=5                      
FL_NUM_GREEN=8                      
FL_NUM_BLUE=5

# label defines
FL_SYMBOL_LABEL=FL_NORMAL_LABEL
FL_SHADOW_LABEL=fl_define_FL_SHADOW_LABEL()
FL_ENGRAVED_LABEL=fl_define_FL_ENGRAVED_LABEL()
FL_EMBOSSED_LABEL=fl_define_FL_EMBOSSED_LABEL()
FL_MULTI_LABEL=FL_EMBOSSED_LABEL+1
FL_ICON_LABEL=FL_MULTI_LABEL+1
FL_IMAGE_LABEL=FL_ICON_LABEL+1

FL_ALIGN_CENTER=0
FL_ALIGN_TOP=1
FL_ALIGN_BOTTOM=2
FL_ALIGN_LEFT=4
FL_ALIGN_RIGHT=8
FL_ALIGN_INSIDE=16
FL_ALIGN_TEXT_OVER_IMAGE=32
FL_ALIGN_IMAGE_OVER_TEXT=0
FL_ALIGN_CLIP=64
FL_ALIGN_WRAP=128
FL_ALIGN_IMAGE_NEXT_TO_TEXT=0x0100
FL_ALIGN_TEXT_NEXT_TO_IMAGE=0x0120
FL_ALIGN_IMAGE_BACKDROP=0x0200
FL_ALIGN_TOP_LEFT= FL_ALIGN_TOP | FL_ALIGN_LEFT
FL_ALIGN_TOP_RIGHT= FL_ALIGN_TOP | FL_ALIGN_RIGHT
FL_ALIGN_BOTTOM_LEFT= FL_ALIGN_BOTTOM | FL_ALIGN_LEFT
FL_ALIGN_BOTTOM_RIGHT= FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT
FL_ALIGN_LEFT_TOP	= 0x0007
FL_ALIGN_RIGHT_TOP	= 0x000b 
FL_ALIGN_LEFT_BOTTOM	= 0x000d 
FL_ALIGN_RIGHT_BOTTOM	= 0x000e 
FL_ALIGN_NOWRAP		= 0 
FL_ALIGN_POSITION_MASK   = 0x000f
FL_ALIGN_IMAGE_MASK      = 0x0320 

# font defines
FL_HELVETICA              = 0	## Helvetica (or Arial) normal (0)
FL_HELVETICA_BOLD         = 1	## Helvetica (or Arial) bold
FL_HELVETICA_ITALIC       = 2	## Helvetica (or Arial) oblique
FL_HELVETICA_BOLD_ITALIC  = 3	## Helvetica (or Arial) bold-oblique
FL_COURIER                = 4	## Courier normal
FL_COURIER_BOLD           = 5	## Courier bold 
FL_COURIER_ITALIC         = 6	## Courier italic
FL_COURIER_BOLD_ITALIC    = 7	## Courier bold-italic
FL_TIMES                  = 8	## Times roman
FL_TIMES_BOLD             = 9	## Times roman bold
FL_TIMES_ITALIC           = 10	## Times roman italic
FL_TIMES_BOLD_ITALIC      = 11	## Times roman bold-italic
FL_SYMBOL                 = 12	## Standard symbol font
FL_SCREEN                 = 13	## Default monospaced screen font
FL_SCREEN_BOLD            = 14	## Default monospaced bold screen font
FL_ZAPF_DINGBATS          = 15	## Zapf-dingbats font

FL_FREE_FONT              = 16	## first one to allocate
FL_BOLD                   = 1	## add this to helvetica, courier, or times
FL_ITALIC                 = 2	## add this to helvetica, courier, or times
FL_BOLD_ITALIC            = 3	## add this to helvetica, courier, or times

FL_COMMAND=fl_command_modifier()
FL_CONTROL=fl_control_modifier()

%}   

