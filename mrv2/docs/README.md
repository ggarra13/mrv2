v0.5.3
------
- Made area selection allow it to select 1 pixel easier by a single click.
  To disable it, you just need to switch to a new action mode (drawing, etc).
- Some users on older macOS versions reported problems with the Privacy
  mechanism of the OS on Documents, Desktop and Download directories.
  The problem is not there if we use the native file chooser.  I've switched
  the default on macOS to use the native file chooser.
- Added a soft brush for annotations on all shapes.  You access it from the
  Annotation panel which can be opened from the menus or by clicking twice on
  any of the draw tools.  The algorithm for smooth brushes is not yet perfect,
  as it can lead to an overlapping triangle on self intersections.
- Allowed splatting a brush stroke if clicking only once.
- Made Pen size in annotations go as low as 2 pixels.  One pixel tends to
  vanish and have issues when panels are open.
- Added license and code attribution to the Polyline2D.h code which was missing
  and I had lost where I downloaded it from.  I have further modified it to
  support UV mapping and indexed triangles.
- Fixed flickering of timeline thumbnail if switched to on first and then
  later set it to off in the preferences.
- Added a session file to store a mrv2 session (.m2s files)
  All files loaded, ui elements, panel values, etc. are saved and restored.
- Fixed a potential crash when using One Panel Only.
- Added Notes to Annotation Panel.  This allows you to add comments on a frame,
  without having to draw anything (or in addition to the drawn elements).
- Made view take the focus upon entering except when typing in the text tool.
- Fixed search in the Hotkey window which was missing the last character of
  the function.
- Fixed search repeatedly in the Hotkey window which was searching from the
  topline instead of from the last selected item.
- Allowed annotation drawing outside of the canvas once again.
- Fixed precision issues on annotation drawings.
- Made annotations respond to R, G, B, A channels changing.
- Removed ngrok documentation as it was incorrect for internet access.
- Fixed resizing of viewport not taking into account the status bar, leading
  to zoom factors of 1/1.04 instead of 1.
