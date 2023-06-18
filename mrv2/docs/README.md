v0.6.0
------
- Added the options for missing frames on the Preferences.  You can now:

  	* Display black
	* Repeat last frame
	* Repeat last frame scratched
	
- Made loading of session files use Path Mapping for files and OCIO config
  so that if a session file is loaded from different OSes the files will be
  found.
- Fixed loading session from the command-line not showing the opened panels that
  were also open in the preferences file.
- Added the name of the layer to the thumnail description in the files, compare,
  playlist and stereo panels.
- Added anaglyph, scanline, columns and checkered stereo 3D.
- Added a new Stereo 3D Panel to control the stereo.

  	* To use it, you load a clip with left and right views (usually a v2
     	  multipart openexr).  Then, open the Files Panel and select the clip
	  and layer to use.
	* Open the Stereo Panel and select the Input to "Image".  That will
	  clone the clip and select the opposite view (ie. right if you
	  selected left).
	* Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).

- You can also use the Stereo 3D Panel with two clips (movies or sequences),
  but you need to set it manually.
  
	* Open the Files Panel, load the two clips. Select one of them.
	* Open the Stereo 3D Panel, select the other clip.  Then select Input as
     	  "Image".
	* Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).
  
- Fixed loading of multiple clips from a session messing up the video layers.
- Made movie's default layer be labeled "Color" to be consistant with images.
- Fixed OpenEXR's v2 multipart images with view (stereo) parameter.
- Fixed OpenEXR's v2 multipart images with changing data windows between frames.
- Fixed mrv2's native file chooser on Windows not cd'ing to the file path
  when the location input field was manually edited.
- Fixed playback starting when session was loaded command line and the session
  was not originally playing.
- Fixed thumbnail display in Files, Compare, Stereo 3D and Playlist panels.
- Fixed order of panels when loaded from a session file.
- Improved performance of exiting the application.
- Made HUD Attributes display the (sometimes changing) frame attributes.
- Added a Data and Display Window display option to the menus and to the
  view window display.
- Added compare and stereo options sent when a client syncs to the server.
- Made File/Clone (Right Mouse Button on Files Panel clip) respect the frame
  and playback state of the original clip.
- Added a File/Refresh Cache (Right Mouse Button on Files Panel clip) to
  refresh the cache.  This is useful when viewing a partially rendered
  sequence.
- Made thumbnails in Files, Compare, Stereo 3D and Playlist panels show the
  actual layer (color channel).
- Made timeline thumbnail reflect the actual layer (color channel).
- Allowed saving annotations to a PDF.  Both picture thumbnails as well as
  text notes are saved.
- Made Media Info Panel refresh every frame when there's a Data Window present.
- Fixed safe areas partially disappearing when zooming out.

