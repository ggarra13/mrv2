#####################
Reproducción de Vídeo
#####################

Cuadro Actual
-------------

En mrv2, cada clip y lista de reproducción tiene su cuadro acutal lo que significa que si cambia de uno a otro, cosas como la velocidad de reproducción, el modo de bucle, etc. son recordadas *por lista de reproducción* o clip. 

Loop Modes
----------

Use the loop mode button to switch between 'play once', 'loop' and 'ping-pong' when playing through media.

FPS Rate
--------

The FPS (Frames per Second) rate can be adjusted by selecting one from the 'FPS' button in the playback toolbar.  You can also type in a random speed in the FPS widget.

Playback Specific Hotkeys
-------------------------

=================  ==============================
Shortcut           Action
=================  ============================== 
Spacebar           Start/stop playback.
I                  Toggle the 'in' loop point.
O                  Toggle the 'out' loop point.
Up Arrow           Play backwards.
Enter              Stop playback.
Down Arrow         Play forwards.
Right Arrow        Step forwards one frame.
Left Arrow         Step backwards one frame.
Shift+Left Arrow   Go to the previous annotation.
Shift+Right Arrow  Go to the next annotation.
=================  ============================== 

Cache Behaviour
---------------

mrv2 will always try to read and decode video data before it is needed for display. The image data is stored in the image cache ready for drawing to screen. mrv2 needs to be efficient in how it does this and it is useful if a user understands the behaviour.

The cache status is indicated in the timeline with a horizontal colored bar - this should be obvious as you can see it growing as mrv2 loads frames in the background. Thus if you want to view media that is slow to read off disk, like high resolution EXR images, the workflow is to wait for mrv2 to cache the frames before starting playback/looping. The size of the cache (set via the Settings Panel) will limit the maximum number of frames that can be loaded. 

In most cases mrv2 should be able to play back through cached frames at the required frame rate of the media. Although the Viewer has been optimised to get the most out of your graphics card, slow playback can result if you are trying to view very high resolution images and your computer's video hardware can't match the required data transfer rates.

For media that can be decoded faster than the playback rate, like many common compressed video stream codecs or EXRs compressed with the DWA/DWB, you should be able to largely ignore mrv2's caching activity as it will be able to stream data off the disk for playback on demand.

