image module
============

.. automodule:: mrv2.image
	       
.. autoclass:: mrv2.image.AlphaBlend
	       
.. autoclass:: mrv2.image.Channels
	       
.. autoclass:: mrv2.image.EnvironmentMapType
	       
.. autoclass:: mrv2.image.ImageFilter
    
.. autoclass:: mrv2.image.InputVideoLevels
	       
.. autoclass:: mrv2.image.LUTOrder
	       
.. autoclass:: mrv2.image.VideoLevels
	       
.. autoclass:: mrv2.image.YUVCoefficients
               
.. autoclass:: mrv2.image.Stereo3DInput
               
.. autoclass:: mrv2.image.Stereo3DOutput
	       
.. autoclass:: mrv2.image.Mirror
	       
   .. autoattribute:: x
      :annotation: = bool
		   
   .. autoattribute:: y
      :annotation: = bool

.. autoclass:: mrv2.image.Color
   
   .. autoattribute:: enabled
      :annotation: = bool
		   
   .. autoattribute:: add
      :annotation: = mrv2.math.Vector3f
		   
   .. autoattribute:: brightness
      :annotation: = mrv2.math.Vector3f
		   
   .. autoattribute:: contrast
      :annotation: = mrv2.math.Vector3f
		   
   .. autoattribute:: saturation
      :annotation: = mrv2.math.Vector3f
		   
   .. autoattribute:: tint
      :annotation: = float
		   
   .. autoattribute:: invert
      :annotation: = bool
		   
.. autoclass:: mrv2.image.Levels
   
   .. autoattribute:: enabled
      :annotation: = bool
		   
   .. autoattribute:: inLow
      :annotation: = float
		   
   .. autoattribute:: inHigh
      :annotation: = float
		   
   .. autoattribute:: gamma
      :annotation: = float
		   
   .. autoattribute:: outLow
      :annotation: = float
		   
   .. autoattribute:: outHigh
      :annotation: = float
		   
.. autoclass:: mrv2.image.ImageFilters
   
   .. autoattribute:: minify
      :annotation: = mrv2.image.ImageFilter
		   
   .. autoattribute:: magnify
      :annotation: = mrv2.image.ImageFilter
		   
.. autoclass:: mrv2.image.SoftClip
   
   .. autoattribute:: enabled
      :annotation: = bool
		   
   .. autoattribute:: value
      :annotation: = float
		   
.. autoclass:: mrv2.image.DisplayOptions

   .. autoattribute:: channels
      :annotation: = mrv2.image.Channels
		   
   .. autoattribute:: mirror
      :annotation: = mrv2.image.Mirror
		   		   
   .. autoattribute:: color
      :annotation: = mrv2.image.Color
		   		   
   .. autoattribute:: levels
      :annotation: = mrv2.image.Levels
		   
   .. autoattribute:: softClip
      :annotation: = mrv2.image.SoftClip

.. autoclass:: mrv2.image.LUTOptions
   
   .. autoattribute:: fileName
      :annotation: = str
		   
   .. autoattribute:: order
      :annotation: = mrv2.image.LUTOrder
		   
.. autoclass:: mrv2.image.ImageOptions
   
   .. autoattribute:: videoLevels
      :annotation: = mrv2.image.InputVideoLevels
		   
   .. autoattribute:: alphaBlend
      :annotation: = mrv2.image.AlphaBlend
		   
   .. autoattribute:: imageFilters
      :annotation: = mrv2.image.ImageFilters
		   
.. autoclass:: mrv2.image.EnvironmentMapOptions
   
   .. autoattribute:: type
      :annotation: = mrv2.image.EnvironmentMapType
		   
   .. autoattribute:: horizontalAperture
      :annotation: = float
		   
   .. autoattribute:: verticalAperture
      :annotation: = float
		   
   .. autoattribute:: focalLength
      :annotation: = float
		   
   .. autoattribute:: rotateX
      :annotation: = float
		   
   .. autoattribute:: rotateY
      :annotation: = float
		   
   .. autoattribute:: subdivisionX
      :annotation: = int
		   
   .. autoattribute:: subdivisionY
      :annotation: = int
		   
   .. autoattribute:: spin
      :annotation: = bool

.. autoclass:: mrv2.image.Stereo3DOptions
   
   .. autoattribute:: input
      :annotation: = mrv2.image.Stereo3DInput
		   
   .. autoattribute:: output
      :annotation: = mrv2.image.Stereo3DOutput
                   
   .. autoattribute:: eyeSeparation
      :annotation: = float
		   
   .. autoattribute:: swapEyes
      :annotation: = bool
