# use pkg-config for getting CFLAGS and LDLIBS
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          

CFLAGS += -Wall -g
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
CFLAGS += $(shell fltk-config --cflags)

LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS)
LDLIBS += $(shell fltk-config --use-gl --ldstaticflags)
LDLIBS += -lc++

# missing the following targets, since they need special options in the FFmpeg build:
# qsv_decode
# qsv_transcode
# vaapi_encode
# vaapi_transcode

EXAMPLES=hw_decode                       

OBJS=$(addsuffix .o,$(EXAMPLES))

# the following examples make explicit use of the math library
avcodec:           LDLIBS += -lm
encode_audio:      LDLIBS += -lmy
mux:               LDLIBS += -lm
resample_audio:    LDLIBS += -lm

.phony: all clean-test clean

all: $(OBJS) $(EXAMPLES)
# Rule for compiling .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling .cxx files
%.o: %.cxx
	$(CXX) $(CFLAGS) -c $< -o $@

clean-test:
	$(RM) test*.pgm test.h264 test.mp2 test.sw test.mpg

clean: clean-test
	$(RM) $(EXAMPLES) $(OBJS)
