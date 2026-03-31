v1.6.1
======

Change Log
----------

- Added Rec2020 support for the Vectorscope.
- Made BT601 and BT709 in Vectorscope work when the swapchain is VK_COLOR_SPACE_HDR10_ST2084_EXT.  There are some minor floating point issues.
- Fixed vectorscope to display the proper image values when pixel bar's pixel types is set to Original (O).
- Added Panel->Waveform Monitor as a new Panel.
- Fixed a bad redraw issue with Vectorscope at Rec2020.
- Made waveform display the proper image values when pixel bar's pixel type is set to Original (O).
- Added SDR and HDR settings to Waveform monitor.
- Fixed some bugs in color formulas.
- Fixed a huge bug from v1.6.0 where it would not allow saving movie files.
- Fixed a huge bug when saving image sequences or movies in Vulkan backend.

