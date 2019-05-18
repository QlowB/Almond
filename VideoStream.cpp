#ifdef FFMPEG_ENABLED

#include "VideoStream.h"

#include <iostream>



#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free  avcodec_free_frame
#endif

const uint8_t VideoStream::endcode[] = { 0, 0, 1, 0xb7 };


VideoStream::VideoStream(::size_t width, ::size_t height, const std::string& filename) :
    width{ width }, height{ height }
{
    avcodec_register_all();

    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!codec) {
        fprintf(stderr, "invalid codec\n");
        exit(1);
    }

    codecContext = avcodec_alloc_context3(codec);
    picture = av_frame_alloc();

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    codecContext->bit_rate = 500000 * 100;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = AVRational{ 1, 60 };
    codecContext->framerate = AVRational{ 60, 1 };

    codecContext->gop_size = 10; /* emit one intra frame every ten frames */
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    file = fopen(filename.c_str(), "wb");
    if (!file) {
        fprintf(stderr, "could not open %s\n", filename.c_str());
        exit(1);
    }

    picture->format = codecContext->pix_fmt;
    picture->width  = codecContext->width;
    picture->height = codecContext->height;

    int retval = av_frame_get_buffer(picture, 0);
    if (retval < 0) {
        fprintf(stderr, "could not alloc the frame data\n");
        exit(1);
    }
    //av_image_alloc(picture->data, picture->linesize, width, height, codecContext->pix_fmt, 32);

    swsContext = sws_getContext(width, height,
        AV_PIX_FMT_RGB24, width, height,
        AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
}


static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
    FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "error during encoding\n");
            exit(1);
        }

        printf("encoded frame %3\"PRId64\" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}


VideoStream::~VideoStream()
{
    /* flush the encoder */
    encode(codecContext, NULL, pkt, file);

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);

    avcodec_free_context(&codecContext);
    av_frame_free(&picture);
    av_packet_free(&pkt);
}


void VideoStream::addFrame(const Bitmap<RGBColor>& frame)
{
    int retval = av_frame_make_writable(picture);
    if (retval < 0)
        exit(1);

    /* prepare a dummy image */
    /* Y */
    /*for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            picture->data[0][y * picture->linesize[0] + x] = frame.get(x, y).r / 2;
        }
    }*/

    /* Cb and Cr */
    /*for(int y=0;y<height / 2;y++) {
        for(int x=0;x<width / 2;x++) {
            picture->data[1][y * picture->linesize[1] + x] = frame.get(x * 2, y * 2).g / 2;
            picture->data[2][y * picture->linesize[2] + x] = frame.get(x * 2, y * 2).b / 2;
        }
    }*/

    const uint8_t* pixelPointer[] = { reinterpret_cast<const uint8_t*>(frame.pixels.get()), 0 };
    const int linesizeIn[] = { int(frame.width * sizeof(RGBColor)) };

    sws_scale(swsContext, pixelPointer, linesizeIn, 0,
        frame.height, picture->data, picture->linesize);

    picture->pts = frameIndex++;

    /* encode the image */
    encode(codecContext, picture, pkt, file);
}

#endif // FFMPEG_ENABLED
