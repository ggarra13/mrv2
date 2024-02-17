This directory contains FFmpeg presets, named as:

codec_presetname.pst

For example:

vp9_good.pst

These presets will be listed in the Save Options when the proper video codec
is used.  Note that the codec name is lowercase.

The preset file is a colon separated file of parameters, like:

```
# This is a comment
deadline:good      # this is another comment
webm:              # flag with no parameters
tile-column:1      # integer flag
cq-level:25
```
