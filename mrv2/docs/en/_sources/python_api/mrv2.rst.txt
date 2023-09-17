mrv2 module
===========

.. automodule:: mrv2
    :members:
    :exclude-members: FileMedia
    
.. autoclass:: mrv2.FileMedia

   .. autoattribute:: path
      :annotation: = mrv2.Path

   .. autoattribute:: audioPath
      :annotation: = mrv2.Path

   .. autoattribute:: timeRange
      :annotation: = mrv2.TimeRange

   .. autoattribute:: playback
      :annotation: = mrv2.timeline.Playback
		   
   .. autoattribute:: loop
      :annotation: = mrv2.timeline.Loop
		   
   .. autoattribute:: currentTime
      :annotation: = mrv2.RationalTime
		   
   .. autoattribute:: inOutRange
      :annotation: = mrv2.TimeRange
		   
   .. autoattribute:: videoLayer
      :annotation: = int
		   
   .. autoattribute:: volume
      :annotation: = float
		   
   .. autoattribute:: mute
      :annotation: = bool
		   
   .. autoattribute:: audioOffset
      :annotation: = float
