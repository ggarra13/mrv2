######
Saving
######

Saving a Movie
--------------
	   
When saving a movie from the menu File->Save Movie or Sequence and typing in the extension of a movie file (like "test.mov") an option window will appear.

.. image:: ./images/save-movie-01.png
   :align: center
	   
.. topic:: Save Annotations

	   When this option is on, annotations in the video will get saved.

.. topic:: Video Options

* Profile

  With the Profile option, you can select the basic codec used when saving the movie.  Currently these can be "None", "H264", "ProRes", "ProRes_Proxy", "ProRes_LT", "ProRes_HQ", "ProRes_4444" or "ProRes_XQ".
Note that the most compatible codec ("H264") requires a license from VideoLAN for encoding or to compile mrv2 in GPL mode.
Please refer to the compilation instructions of mrv2 at:
	      
             https:://github.com/ggarra13/mrv2

* Preset

  With the Preset selection, you can choose the quality of encoding with the selected codec.  Currently, we ship presets for vp9 and av1.
  You can place them in $STUDIOPATH/presets or in the presets
  directory of the mrv2 installation.

  The presets should be named like::
	      
    codec_presetname.pst

  For example::

    vp9_good.pst

  These presets will be listed in the Save Options when the
  proper video codec is used.  Note that the codec name is
  lowercase.

  The preset file is a colon separated file of parameters, like::

    # This is a comment
    deadline:good      # this is another comment
    webm:              # flag with no parameters
    tile-column:1      # integer flag
    cq-level:25


.. topic:: Audio Options

	   * Codec

	      With this setting, you will be able to control the codec used
	      when saving a movie file with audio.
	      

Saving a Sequence of Images
---------------------------
	   
When saving a sequence of pictures from the menu File->Save Movie or Sequence and typing in the extension of a sequence with a number (like "test.0001.exr") an option window will appear.

.. image:: ./images/save-images-01.png
   :align: center
	   
.. topic:: Save Annotations

	   When this option is on, annotations in the video will get saved.
	   This option will also allow you to save formats that are incompatible
	   with OpenEXR, for example, such as the YUV formats of most movies.

.. topic:: OpenEXR Options

	   * Compression

	      This setting allows you to select the type of compression used
	      when saving the sequence of OpenEXRs.

	   * Pixel Type

	      You can select the type of pixel type to save (Half or Float).

	   * ZIP Compression

	      When saving the compression type "ZIP" or "ZIPS" this setting
	      allows you to control the amount of zip compression.

	   * DWA Compression

	      For DWAA and DWAB compression, this setting controls the amount
	      of compression in those formats.
