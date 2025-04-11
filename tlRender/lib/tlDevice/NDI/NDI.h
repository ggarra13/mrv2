#pragma once

// \@bug Apple's SDK is broken on new macOS.
#undef NULL
#define NULL nullptr
#include <Processing.NDI.Lib.h>

// Structs
#define NDIlib_audio_frame_t NDIlib_audio_frame_v3_t
#define NDIlib_video_frame_t NDIlib_video_frame_v2_t
#define NDIlib_recv_create_t NDIlib_recv_create_v3_t

// Find functions
#define NDIlib_find_create NDIlib_find_create_v2

// Receive Functions
#define NDIlib_recv_create NDIlib_recv_create_v3
#define NDIlib_recv_capture NDIlib_recv_capture_v3
#define NDIlib_recv_free_video NDIlib_recv_free_video_v2
#define NDIlib_recv_free_audio NDIlib_recv_free_audio_v3
#define NDIlib_util_audio_to_interleaved_16s                                   \
    NDIlib_util_audio_to_interleaved_16s_v2

// Send functions
#define NDIlib_send_send_video NDIlib_send_send_video_v2
#define NDIlib_send_send_audio NDIlib_send_send_audio_v3

#define NDI_TIME_BASE 10000000
#define NDI_TIME_BASE_Q                                                        \
    (AVRational)                                                               \
    {                                                                          \
        1, NDI_TIME_BASE                                                       \
    }

#define kNDI_MOVIE_DURATION 60.0 * 60.0 * 3.0 // in seconds (3 hours)
