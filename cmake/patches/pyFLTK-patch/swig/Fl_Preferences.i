/* File : Fl_Preferences.i */
//%module Fl_Preferences

%feature("docstring") ::Fl_Preferences
"""
Fl_Preferences provides methods to store user setting between application
starts. It is similar to the Registry on WIN32 and Preferences on MacOS,
and provides a simple configuration mechanism for UNIX.

Fl_Preferences uses a hierarchy to store data. It bundles similar data into
groups and manages entries into those groups as name/value pairs.

Preferences are stored in text files that can be edited manually. The file
format is easy to read and relatively forgiving. Preferences files are the
same on all platforms. User comments in preference files are preserved.
Filenames are unique for each application by using a vendor/application
naming scheme. The user must provide default values for all entries to
ensure proper operation should preferences be corrupted or not yet exist.

Entries can be of any length. However, the size of each preferences file
should be kept under 100k for performance reasons. One application can
have multiple preferences files. Extensive binary data however should be
stored in seperate files; see the getUserdataPath() method.
""" ;

%{
#include "FL/Fl_Preferences.H"
%}

%ignore Fl_Preferences::Fl_Preferences( Fl_Preferences*, const char *group );
%ignore Fl_Preferences::Fl_Preferences( const char *path, const char *vendor, const char *application );
%ignore Fl_Preferences::set( const char *entry, float value );

//%include "FL/Fl_Preferences.H"

%include "cstring.i"

//%cstring_bounded_output(char *value, 1024);
//%apply char *STRING {char *value};


// typemap to map output of Fl_Button.value from char to int
%typemap(out) char {
    $result = PyLong_FromLong( (long)$1);
}
class FL_EXPORT Fl_Preferences
{

public:

  enum Root { SYSTEM=0, USER };
  // enum Type { win32, macos, fltk };

  Fl_Preferences( Root root, const char *vendor, const char *application );
  Fl_Preferences( const char *path, const char *vendor, const char *application );
  Fl_Preferences( Fl_Preferences&, const char *group );
  //Fl_Preferences( Fl_Preferences*, const char *group );
  ~Fl_Preferences();

  int groups();
  const char *group( int );
  char groupExists( const char *group );
  char deleteGroup( const char *group );

  int entries();
  const char *entry( int );
  char entryExists( const char *entry );
  char deleteEntry( const char *entry );

  char set( const char *entry, int value );
  char set( const char *entry, float value );
  char set( const char *entry, float value, int precision );
  char set( const char *entry, double value );
  char set( const char *entry, double value, int precision );
  char set( const char *entry, const char *cvalue );
  char set( const char *entry, const void *cvalue, int size );

  char get( const char *entry, int &OUTPUT,    int defaultValue );
  char get( const char *entry, float &OUTPUT,  float defaultValue );
  char get( const char *entry, double &OUTPUT, double defaultValue );
%cstring_bounded_output(char *value, 1024);
  //char get( const char *entry, char *&value,  const char *defaultValue );
  char get( const char *entry, char *value,   const char *defaultValue, int maxSize );
  //char get( const char *entry, void *&value,  const void *defaultValue, int defaultSize );
  char get( const char *entry, void *value,   const void *defaultValue, int defaultSize, int maxSize );

  int size( const char *entry );


%cstring_output_maxsize(char *path, int pathlen);
  char getUserdataPath( char *path, int pathlen );

  void flush();
};


// clear the typemap for char
%typemap(out) char;
