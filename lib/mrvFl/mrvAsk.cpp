#include <stdio.h>
#include <stdarg.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>

#include <FL/platform.H>
#include <FL/fl_draw.H>

extern "C" {
#include <libavutil/mem.h>
}

namespace mrv
{

static Fl_Window *message_form;
static Fl_Box *message;
static Fl_Box *icon;
static Fl_Button *button[3];
static Fl_Input *input;
static int ret_val;
static const char *iconlabel = "?";
static const char *message_title_default;
Fl_Font fl_message_font_ = FL_HELVETICA;
Fl_Fontsize fl_message_size_ = -1;
static int enableHotspot = 1;

static char avoidRecursion = 0;

// Sets the global return value (ret_val) and closes the window.
// Note: this is used for the button callbacks and the window
// callback (closing the window with the close button or menu).
// The first argument (Fl_Widget *) can either be an Fl_Button*
// pointer to one of the buttons or an Fl_Window* pointer to the
// message window (message_form).
static void button_cb(Fl_Widget *, long val) {
  ret_val = (int) val;
  message_form->hide();
}

static Fl_Window *makeform() {
 if (message_form) {
   return message_form;
 }
 // make sure that the dialog does not become the child of some
 // current group
 Fl_Group *previously_current_group = Fl_Group::current();
 Fl_Group::current(0);
 // create a new top level window
 Fl_Window *w = message_form = new Fl_Window(410,103);
  message_form->callback(button_cb);
 Fl_Group* g = new Fl_Group( 0, 0, 410, 103 );
 g->box( FL_UP_BOX );
 // w->clear_border();
 (message = new Fl_Box(60, 25, 340, 30))
 ->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
 message->box(FL_UP_BOX);
 (input = new Fl_Input(60, 37, 340, 23))->hide();
 {Fl_Box* o = icon = new Fl_Box(10, 10, 50, 50);
  o->box(FL_THIN_UP_BOX);
  o->labelfont(FL_TIMES_BOLD);
  o->labelsize(34);
  o->color(FL_WHITE);
  o->labelcolor(FL_BLUE);
 }
 g->end();
 w->end(); // don't add the buttons automatically
 // create the buttons (right to left)
 {
   for (int b=0, x=310; b<3; b++, x -= 100) {
     if (b==1)
       button[b] = new Fl_Return_Button(x, 70, 90, 23);
     else
       button[b] = new Fl_Button(x, 70, 90, 23);
     button[b]->align(FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
     button[b]->callback(button_cb, b);
   }
 }
 button[0]->shortcut(FL_Escape);
 // add the buttons (left to right)
 {
   for (int b=2; b>=0; b--)
     w->add(button[b]);
 }
 w->begin();
 w->resizable(new Fl_Box(60,10,110-60,27));
 w->end();
 w->set_modal();
 Fl_Group::current(previously_current_group);
 return w;
}

/*
 * 'resizeform()' - Resize the form and widgets so that they hold everything
 *                  that is asked of them...
 */

static void resizeform() {
  int	i;
  int	message_w, message_h;
  int	text_height;
  int	button_w[3], button_h[3];
  int	x, w, h, max_w, max_h;
        const int icon_size = 50;

  message_form->size(410,103);

  fl_font(message->labelfont(), message->labelsize());
  message_w = message_h = 0;
  fl_measure(message->label(), message_w, message_h);

  message_w += 10;
  message_h += 10;
  if (message_w < 340)
    message_w = 340;
  if (message_h < 30)
    message_h = 30;

  fl_font(button[0]->labelfont(), button[0]->labelsize());

  memset(button_w, 0, sizeof(button_w));
  memset(button_h, 0, sizeof(button_h));

  for (max_h = 25, i = 0; i < 3; i ++)
    if (button[i]->visible())
    {
      fl_measure(button[i]->label(), button_w[i], button_h[i]);

      if (i == 1)
        button_w[1] += 20;

      button_w[i] += 30;
      button_h[i] += 10;

      if (button_h[i] > max_h)
        max_h = button_h[i];
    }

  if (input->visible()) text_height = message_h + 25;
  else text_height = message_h;

  max_w = message_w + 10 + icon_size;
  w     = button_w[0] + button_w[1] + button_w[2] - 10;

  if (w > max_w)
    max_w = w;

  message_w = max_w - 10 - icon_size;

  w = max_w + 20;
  h = max_h + 30 + text_height;

  message_form->size(w, h);
  message_form->size_range(w, h, w, h);

  message->resize(20 + icon_size, 10, message_w, message_h);
  icon->resize(10, 10, icon_size, icon_size);
  icon->labelsize(icon_size - 10);

  for (x = w, i = 0; i < 3; i ++)
    if (button_w[i])
    {
      x -= button_w[i];
      button[i]->resize(x, h - 10 - max_h, button_w[i] - 10, max_h);

//      printf("button %d (%s) is %dx%d+%d,%d\n", i, button[i]->label(),
//             button[i]->w(), button[i]->h(),
//	     button[i]->x(), button[i]->y());
    }
  message_form->init_sizes();
}

static int innards(const char* fmt, va_list ap,
  const char *b0,
  const char *b1,
  const char *b2)
{
  Fl::pushed(0); // stop dragging (STR #2159)

  avoidRecursion = 1;

  makeform();
  message_form->size(410,103);
  char buffer[1024];
  if (!strcmp(fmt,"%s")) {
    message->label(va_arg(ap, const char*));
  } else {
    ::vsnprintf(buffer, 1024, fmt, ap);
    message->label(buffer);
  }

  message->labelfont(fl_message_font_);
  if (fl_message_size_ == -1)
    message->labelsize(FL_NORMAL_SIZE);
  else
    message->labelsize(fl_message_size_);
  if (b0) {button[0]->show(); button[0]->label(b0); button[1]->position(210,70);}
  else {button[0]->hide(); button[1]->position(310,70);}
  if (b1) {button[1]->show(); button[1]->label(b1);}
  else button[1]->hide();
  if (b2) {button[2]->show(); button[2]->label(b2);}
  else button[2]->hide();
  const char* prev_icon_label = icon->label();
  if (!prev_icon_label) icon->label(iconlabel);

  resizeform();

  if (button[1]->visible() && !input->visible() )
    button[1]->take_focus();
  if (enableHotspot)
    message_form->hotspot(button[0]);
  if (b0 && Fl_Widget::label_shortcut(b0))
    button[0]->shortcut(0);
  else
    button[0]->shortcut(FL_Escape);

  // set default window title, if defined and a specific title is not set
  if (!message_form->label() && message_title_default)
    message_form->label(message_title_default);

  // deactivate Fl::grab(), because it is incompatible with modal windows
  Fl_Window* g = Fl::grab();
  if (g) Fl::grab(0);
  Fl_Group *current_group = Fl_Group::current(); // make sure the dialog does not interfere with any active group
  message_form->show();
  Fl_Group::current(current_group);
  while (message_form->shown()) Fl::wait();
  if (g) // regrab the previous popup menu, if there was one
    Fl::grab(g);
  icon->label(prev_icon_label);
  message_form->label(0); // reset window title

  avoidRecursion = 0;
  return ret_val;
}

 /** \addtogroup group_comdlg
    @{ */

// pointers you can use to change FLTK to another language:
const char* fl_no = "No";        ///< string pointer used in common dialogs, you can change it to another language
const char* fl_yes= "Yes";       ///< string pointer used in common dialogs, you can change it to another language
const char* fl_ok = "OK";        ///< string pointer used in common dialogs, you can change it to another language
const char* fl_cancel= "Cancel"; ///< string pointer used in common dialogs, you can change it to another language
const char* fl_close= "Close";   ///< string pointer used in common dialogs, you can change it to another language

// fltk functions:

/** Shows a dialog displaying the printf style \p fmt message,
    this dialog features up to 3 customizable choice buttons
    which are specified in order of *right-to-left* in the dialog, e.g.
    \image html  fl_choice_left_middle_right.png
    \image latex fl_choice_left_middle_right.png  "fl_choice() button ordering" width=4cm

   \note Common dialog boxes are application modal. No more than one common dialog box
    can be open at any time. Requests for additional dialog boxes are ignored.
   \note \#include <FL/fl_ask.H>

   Three choices with printf() style formatting:
   \image html  fl_choice_three_fmt.png
   \image latex fl_choice_three_fmt.png  "fl_choice() three choices with printf formatting" width=4cm
   \code
       int num_msgs = GetNumberOfMessages();
       switch ( fl_choice("What to do with %d messages?", "Send", "Save", "Delete", num_msgs) ) {
         case 0: .. // Send
         case 1: .. // Save (default)
         case 2: .. // Delete
         ..
       }
   \endcode

   Three choice example:
   \image html  fl_choice_three.png
   \image latex fl_choice_three.png  "fl_choice() three choices" width=4cm
   \code
   switch ( fl_choice("How many bedrooms?", "Zero", "One", "Two") ) {
     case 0: .. // "Zero"
     case 1: .. // "One" (default)
     case 2: .. // "Two"
   }
   \endcode

   Two choice example:
   \image html  fl_choice_two.png
   \image latex fl_choice_two.png  "fl_choice() two choices" width=4cm
   \code
       switch ( fl_choice("Empty trash?", "Yes", "No", 0) ) {
         case 0: .. // Yes
         case 1: .. // No (default)
       }
   \endcode

   One choice example:
   \image html  fl_choice_one.png
   \image latex fl_choice_one.png  "fl_choice() one choice" width=4cm
   \code
       fl_choice("All hope is lost.", "OK", 0, 0);   // "OK" default
   \endcode

   \param[in] fmt can be used as an sprintf-like format and variables for the message text
   \param[in] b0 text label for right button 0
   \param[in] b1 text label for middle button 1 (can be 0)
   \param[in] b2 text label for left button 2 (can be 0)
   \retval 0 if the button with \p b0 text is pushed or another dialog box is still open
   \retval 1 if the button with \p b1 text is pushed
   \retval 2 if the button with \p b2 text is pushed
 */
int fl_choice(const char*fmt,const char *b0,const char *b1,const char *b2,...){

  if (avoidRecursion) return 0;

  va_list ap;

  // fl_beep(FL_BEEP_QUESTION);

  va_start(ap, b2);
  int r = innards(fmt, ap, b0, b1, b2);
  va_end(ap);
  return r;
}

static const char* input_innards(const char* fmt, va_list ap,
                                 const char* defstr, uchar type) {
  makeform();
  message_form->size(410,103);
  message->position(60,10);
  input->type(type);
  input->show();
  input->value(defstr);
  input->take_focus();

  int r = innards(fmt, ap, fl_cancel, fl_ok, 0);
  input->hide();
  message->position(60,25);
  return r ? input->value() : 0;
}

const char* fl_input(const char *fmt, const char *defstr, ...) {

  if (avoidRecursion) return 0;

  va_list ap;
  va_start(ap, defstr);
  const char* r = input_innards(fmt, ap, defstr, FL_NORMAL_INPUT);
  va_end(ap);
  return r;
}

/** Shows an alert message dialog box

   \note Common dialog boxes are application modal. No more than one common dialog box
   can be open at any time. Requests for additional dialog boxes are ignored.
   \note \#include <FL/fl_ask.H>

   \param[in] fmt can be used as an sprintf-like format and variables for the message text
 */
void fl_alert(const char *fmt, ...) {

  if (avoidRecursion) return;

  va_list ap;

  // fl_beep(FL_BEEP_ERROR);

  va_start(ap, fmt);
  iconlabel = "!";
  innards(fmt, ap, 0, fl_close, 0);
  va_end(ap);
  iconlabel = "?";
}

/** Sets whether or not to move the common message box used in
    many common dialogs like fl_message(), fl_alert(),
    fl_ask(), fl_choice(), fl_input(), fl_password() to follow
    the mouse pointer.

    The default is \e enabled, so that the default button is the
    hotspot and appears at the mouse position.
    \note \#include <FL/fl_ask.H>
    \param[in]	enable	non-zero enables hotspot behavior,
                        0 disables hotspot
 */
void fl_message_hotspot(int enable) {
  enableHotspot = enable ? 1 : 0;
}

/** Sets the title of the dialog window used in many common dialogs.

    This window \p title will be used in the next call of one of the
    common dialogs like fl_message(), fl_alert(), fl_ask(), fl_choice(),
    fl_input(), fl_password().

    The \p title string is copied internally, so that you can use a
    local variable or free the string immediately after this call. It
    applies only to the \b next call of one of the common dialogs and
    will be reset to an empty title (the default for all dialogs) after
    that call.

    \note \#include <FL/fl_ask.H>
    \param[in] title	window label, string copied internally
*/
void fl_message_title(const char *title) {
  makeform();
  message_form->copy_label(title);
}

/** Sets the default title of the dialog window used in many common dialogs.

    This window \p title will be used in all subsequent calls of one of the
    common dialogs like fl_message(), fl_alert(), fl_ask(), fl_choice(),
    fl_input(), fl_password(), unless a specific title has been set
    with fl_message_title(const char *title).

    The default is no title. You can override the default title for a
    single dialog with fl_message_title(const char *title).

    The \p title string is copied internally, so that you can use a
    local variable or free the string immediately after this call.

    \note \#include <FL/fl_ask.H>
    \param[in] title	default window label, string copied internally
*/
void fl_message_title_default(const char *title) {
  if (message_title_default) {
      av_free ((void *)message_title_default);
      message_title_default = 0;
  }
  if (title)
    message_title_default = av_strdup(title);
}

} // namespace mrv

/** @} */

//
// End of "$Id$".
//
