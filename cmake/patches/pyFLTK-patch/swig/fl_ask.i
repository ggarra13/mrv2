/* File : fl_ask.i */
//%module fl_ask

%{
#include "FL/fl_ask.H"
%}

//%ignore fl_alert;
//%ignore fl_ask;
//%ignore fl_choice;
%ignore fl_input;
//%ignore fl_message;
%ignore fl_password;
%ignore fl_input_str;
%ignore fl_password_str;

%wrapper %{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
%}
%include "FL/fl_ask.H"
%wrapper %{
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
%}


// avoiding varargs problem
%rename (fl_password) fl_vararg_password;
%rename (fl_input) fl_vararg_input;


%ignore fl_input_str(int maxchar, const char *label, const char *deflt = 0, ...);
%ignore fl_input_str(int &ret, int maxchar, const char *label, const char *deflt = 0, ...);
%ignore fl_password_str(int maxchar, const char *label, const char *deflt = 0, ...);
%ignore fl_password_str(int &ret, int maxchar, const char *label, const char *deflt = 0, ...);


%inline %{
  const char *fl_vararg_input(const char *label, const char *deflt = 0) {
    const char* result = 0;
    result = fl_input("%s", deflt, label);
    return result;
  };

  const char *fl_vararg_password(const char *label, const char *deflt = 0) {
    const char* result = 0;
    result = fl_password("%s", deflt, label);
    return result;
  };

  const char* fl_no_get() {
    return fl_yes;
  };
  const char* fl_yes_get() {
    return fl_yes;
  };
 const char* fl_ok_get() {
    return fl_yes;
 };
 const char* fl_cancel_get() {
    return fl_yes;
 };
 const char* fl_close_get() {
    return fl_yes;
 };
 void fl_no_set(const char* value) {
   fl_no = value;
 };
 void fl_yes_set(const char* value) {
   fl_yes = value;
 };
 void fl_ok_set(const char* value) {
   fl_ok = value;
 };
 void fl_cancel_set(const char* value) {
   fl_cancel = value;
 };
 void fl_close_set(const char* value) {
   fl_close = value;
 };
%}

// multi-threading extensions
//%rename (fl_message) fl_mt_message;
//%rename (fl_alert) fl_mt_alert;
//%rename (fl_ask) fl_mt_ask;
//%rename (fl_choice) fl_mt_choice;
//%rename (fl_input) fl_mt_input;
//%rename (fl_password) fl_mt_password;
%inline %{
  void fl_mt_message(const char* text) {
    Py_BEGIN_ALLOW_THREADS;
    fl_message("%s", text);
    Py_END_ALLOW_THREADS;
  };

  void fl_mt_alert(const char* text) {
    Py_BEGIN_ALLOW_THREADS;
    fl_alert("%s", text);
    Py_END_ALLOW_THREADS;
  };

  
  // int fl_mt_ask(const char* text) {
  //   int status = 0;
  //   #ifdef __GNUC__
  //   #pragma GCC diagnostic push
  //   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  //   #endif
  //   Py_BEGIN_ALLOW_THREADS;
  //   status = fl_ask("%s", text);
  //   Py_END_ALLOW_THREADS;
  //   #ifdef __GNUC__
  //   #pragma GCC diagnostic pop
  //   #endif
  //   return status;
  // };

  int fl_mt_choice(const char *q,const char *b0,const char *b1,const char *b2) {
    int status = 0;
    Py_BEGIN_ALLOW_THREADS;
    status = fl_choice("%s", b0, b1, b2, q);
    Py_END_ALLOW_THREADS;
    return status;
  };

  const char *fl_mt_input(const char *label, const char *deflt = 0) {
    const char* result = 0;
    Py_BEGIN_ALLOW_THREADS;
    result = fl_input("%s", deflt, label);
    Py_END_ALLOW_THREADS;
    return result;
  };

  const char *fl_mt_password(const char *label, const char *deflt = 0) {
    const char* result = 0;
    Py_BEGIN_ALLOW_THREADS;
    result = fl_password("%s", deflt, label);
    Py_END_ALLOW_THREADS;
    return result;
  };

%}
