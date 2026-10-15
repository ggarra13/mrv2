# Hardware video decoding

On macOS, FFmpeg playback tries VideoToolbox for ProRes, H.264 (AVC), and
H.265 (HEVC). Linux and Windows default to software decoding; upstream's
other hardware backends remain available when supported by the FFmpeg build.
Availability depends on the FFmpeg build, operating system, hardware, and the
video profile.

In **Settings > Performance**, the existing **FFmpeg HW Accel** switch enables
this behavior (on by default on macOS). **FFmpeg HW Driver** selects a backend.
Turning hardware acceleration off reloads the current movie with the software
decoder. The persisted switch is `Performance/FFmpegHWAccel` (`1` or `0`);
the tlRender reader options are `FFmpeg/HWAccel` (`"1"` or `"0"`) and
`FFmpeg/HWDriver` (e.g. `"videotoolbox"`). The original `FFmpeg/HardwareDecode`
reader option remains an alias; an explicit `FFmpeg/HWAccel` takes precedence.
These options affect playback and are independent of `FFmpeg/HardwareEncode`
for export.

Hardware frames are downloaded to system memory and converted to the existing
image layout. Upstream's native semi-planar output is retained, while the
RGB conversion setting, bit depth, timestamps, color information, and HDR
metadata are preserved. This implementation does not import hardware surfaces
directly into the renderer.

If hardware initialization, decoding, or downloading fails, the reader returns
to software decoding and restarts from a keyframe at the requested time.
FFmpeg can also select a software pixel format when a VideoToolbox session
cannot be initialized. Downloaded formats that reduce bit depth or chroma
resolution are rejected. ProRes with alpha uses software decoding: testing
found chroma differences in VideoToolbox's packed ProRes 4444 alpha output.
ProRes 4444 without alpha can use hardware decoding.

The first decoded frame records `Video Decode Backend` as `VideoToolbox` or
`Software`, and logs changes to the selected backend. Before the first frame,
file info may show `Hardware (pending)`; that indicates preparation, not a
successfully decoded hardware frame. The backend describes the FFmpeg output
path; it is not a measurement of GPU or media-engine utilization.

## Build and test

The macOS superbuild already enables the VideoToolbox H.264, HEVC, and ProRes
hardware accelerators in
`tlRender/etc/SuperBuild/cmake/Modules/BuildFFmpeg.cmake`. Rebuild tlRender and
mrv2 with the usual build procedure. No additional FFmpeg build flags are
needed when using that configuration.

A standalone regression test can use an existing dependency installation:

```sh
cmake -S tlRender -B BUILD-decode-test -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_PREFIX_PATH=/absolute/path/to/dependency/install \
  -DTLRENDER_TESTS=OFF -DTLRENDER_FFMPEG_DECODE_TESTS=ON
cmake --build BUILD-decode-test --target tlffmpeg-decode-test
ctest --test-dir BUILD-decode-test -R FFmpegHardwareDecode --output-on-failure
```

The tests require Python 3 and an `ffmpeg` command with `libx264`, `libx265`,
and `prores_ks` encoders. Fixtures are generated in a temporary directory.
Tests compare software and automatic decoding for ProRes 422/4444, ProRes alpha,
H.264 8/10-bit, H.265 8/10-bit HDR and 4:4:4, and an unrelated software codec.
They exercise sequential reads, forward/backward seeks, end-of-stream draining,
RGB and planar output, memory I/O, different thread counts, and image lifetime.
On macOS, a separate test executable injects hardware-device and frame-download
failures to verify software fallback, including a failure after playback starts.

On a machine expected to support the standard hardware fixtures, require the
VideoToolbox backend explicitly:

```sh
python3 tlRender/tests/FFmpegDecodeTest/run.py \
  --decoder BUILD-decode-test/tests/FFmpegDecodeTest/tlffmpeg-decode-test \
  --require-hardware
```
