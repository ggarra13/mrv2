v0.6.3
------
- Added a python plug-in system which is now documented in the
  Help->Documentation.  The environment variable used to look up plug-ins is:

  	MRV2_PYTHON_PLUGINS

  It is a list of colon (Linux or macOS) or semi-colon (Windows) paths.
  Plug-ins are defined, like:

      class Plugin:
          def hello(self):
              print("Hello from plug-in!")

          def menus(self):
              menus = { ("New Menu/Hello", self.hello) }
              return menus

     	

- Added a mrv2_hello.py plug-in for demo purposes.
- Fixed a bug in the log panel appearing compressed on start up when docked.
- Allowed creation of .otio files of a single clip in Playlist Panel.
- Fixed scratched frames showing up on .otio files with gaps in them.
- Fixed preferences not hiding the different bars anymore (regression in
  v0.6.2).
