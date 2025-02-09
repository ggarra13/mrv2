extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
}

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <iostream>

// === FLTK Window Class ===
class VideoWindow : public Fl_Gl_Window {
public:
    uint8_t *rgb_buffer = nullptr;
    int frame_width = 0, frame_height = 0;

    VideoWindow(int W, int H) : Fl_Gl_Window(W, H) {}

    void draw() override {
        if (!rgb_buffer) return;
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i(-1, 1);
        glPixelZoom(1.0f, -1.0f);
        glDrawPixels(frame_width, frame_height, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer);
    }

    void update_frame(uint8_t *buffer, int w, int h) {
        rgb_buffer = buffer;
        frame_width = w;
        frame_height = h;
        redraw();
    }
};

VideoWindow *win = nullptr;

static AVBufferRef *hw_device_ctx = NULL;
struct SwsContext *sws_ctx = nullptr;
static enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      NULL, NULL, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

// === Frame Display ===
void display_frame(AVFrame *frame) {
    if (!sws_ctx) {
        sws_ctx = sws_getContext(frame->width, frame->height,
                                 (AVPixelFormat)frame->format,
                                 frame->width, frame->height, AV_PIX_FMT_RGB24,
                                 SWS_BICUBIC, NULL, NULL, NULL);
        if (!sws_ctx) {
            std::cerr << "Failed to create SwsContext!" << std::endl;
            return;
        }
    }

    int rgb_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, frame->width, frame->height, 1);
    uint8_t *rgb_buffer = (uint8_t *)av_malloc(rgb_size);
    uint8_t *dst_data[4] = { rgb_buffer, NULL, NULL, NULL };
    int dst_linesize[4] = { 3 * frame->width, 0, 0, 0 };

    // Scale the frame
    if (sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dst_data, dst_linesize) <= 0) {
        std::cerr << "sws_scale failed!" << std::endl;
        av_free(rgb_buffer);
        return;
    }

    win->update_frame(rgb_buffer, frame->width, frame->height);
    Fl::check();  // Keep the FLTK window responsive

    av_free(rgb_buffer);
}

// === Updated decode_write() with HW Frame Handling ===
static int decode_write(AVCodecContext *avctx, AVPacket *packet) {
    AVFrame *frame = av_frame_alloc();
    AVFrame *sw_frame = av_frame_alloc();
    int ret;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) return ret;

    while (ret >= 0) {
        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

        AVFrame *disp_frame = frame;

        // Handle HW-accelerated frames
        if (avctx->hw_device_ctx && frame->format == hw_pix_fmt) {
            if (av_hwframe_transfer_data(sw_frame, frame, 0) < 0) {
                std::cerr << "HW frame transfer failed!" << std::endl;
                continue;
            }
            disp_frame = sw_frame;  // Use the software frame
        } else if (frame->format == AV_PIX_FMT_VIDEOTOOLBOX) {
            std::cerr << "Cannot process VideoToolbox frames without transfer!" << std::endl;
            continue;
        }

        display_frame(disp_frame);
    }

    av_frame_free(&frame);
    av_frame_free(&sw_frame);
    return ret;
}

// === Main Function ===
int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <hw_device_type> <input_file>" << std::endl;
        return -1;
    }

    int i;
    const char *device_type = argv[1];
    const char *input_file = argv[2];
    AVFormatContext *input_ctx = nullptr;
    int video_stream, ret;
    AVStream *video = NULL;
    AVCodecContext *decoder_ctx = nullptr;
    const AVCodec *decoder = nullptr;
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Failed to allocate AVPacket\n");
        return -1;
    }

    enum AVHWDeviceType hw_type = av_hwdevice_find_type_by_name(device_type);
    if (hw_type == AV_HWDEVICE_TYPE_NONE) {
        fprintf(stderr, "Device type %s is not supported.\n", argv[1]);
        fprintf(stderr, "Available device types:");
        while((hw_type = av_hwdevice_iterate_types(hw_type)) != AV_HWDEVICE_TYPE_NONE)
            fprintf(stderr, " %s", av_hwdevice_get_type_name(hw_type));
        fprintf(stderr, "\n");
        return -1;
    }
    
    /* open the input file */
    if (avformat_open_input(&input_ctx, input_file, NULL, NULL) != 0) {
        fprintf(stderr, "Cannot open input file '%s'\n", argv[2]);
        return -1;
    }

    if (avformat_find_stream_info(input_ctx, NULL) < 0) {
        fprintf(stderr, "Cannot find input stream information.\n");
        return -1;
    }

    /* find the video stream information */
    ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (ret < 0) {
        fprintf(stderr, "Cannot find a video stream in the input file\n");
        return -1;
    }
    video_stream = ret;
    
    for (i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
        if (!config) {
            fprintf(stderr, "Decoder %s does not support device type %s.\n",
                    decoder->name, av_hwdevice_get_type_name(hw_type));
            return -1;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == hw_type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }
    
    if (!(decoder_ctx = avcodec_alloc_context3(decoder)))
        return AVERROR(ENOMEM);
    
    video = input_ctx->streams[video_stream];
    if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
        return -1;
    
    decoder_ctx->get_format  = get_hw_format;
    
    if (hw_decoder_init(decoder_ctx, hw_type) < 0)
        return -1;

    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
        fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream);
        return -1;
    }

    win = new VideoWindow(800, 600);
    win->show();

    while (av_read_frame(input_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream)
            decode_write(decoder_ctx, packet);
        av_packet_unref(packet);
    }
    
    /* flush the decoder */
    ret = decode_write(decoder_ctx, NULL);

    av_packet_free(&packet);
    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);
    av_buffer_unref(&hw_device_ctx);

    return Fl::run();
}
