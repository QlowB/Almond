#include "VideoStream.h"

#ifdef FFMPEG_ENABLED


#include <iostream>



#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free  avcodec_free_frame
#endif

const uint8_t VideoStream::endcode[] = { 0, 0, 1, 0xb7 };


VideoStream::VideoStream(int width, int height, const std::string& filename) :
    width{ width & (~1) }, height{ height & (~1) }
{
    // only needed with ffmpeg version < 4
    //avcodec_register_all();

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "invalid codec\n");
        exit(1);
    }

    codecContext = avcodec_alloc_context3(codec);

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    codecContext->bit_rate = 50 * 1000 * 1000;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = AVRational{ 1, 60 };
    codecContext->framerate = AVRational{ 60, 1 };

    codecContext->gop_size = 5; /* emit one intra frame every five frames */
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(codecContext->priv_data, "preset", "slow", 0);

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    file = fopen(filename.c_str(), "wb");
    if (!file) {
        fprintf(stderr, "could not open %s\n", filename.c_str());
        exit(1);
    }

    picture = av_frame_alloc();
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

        printf("encoded frame %3d\"PRId64\" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}


VideoStream::~VideoStream()
{
    /* flush the encoder */
    encode(codecContext, nullptr, pkt, file);

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);

    avcodec_free_context(&codecContext);
    av_frame_free(&picture);
    av_packet_free(&pkt);

/*
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    for (;;) {
        avcodec_send_frame(codecContext, NULL);
        if (avcodec_receive_packet(codecContext, &pkt) == 0) {
            av_interleaved_write_frame(codecContext, &pkt);
            av_packet_unref(&pkt);
        }
        else {
            break;
        }
    }

    av_write_trailer();
    if (!(oformat->flags & AVFMT_NOFILE)) {
        int err = avio_close(ofctx->pb);
        if (err < 0) {
            Debug("Failed to close file", err);
        }
    }*/

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

    /*auto gammaCorrect = [] (const RGBColor& rgb) {
        const float gamma = 2.2f;
        return RGBColor {
            uint8_t(::powf(rgb.r / 255.0f, 1.0f / gamma) * 255),
            uint8_t(::powf(rgb.g / 255.0f, 1.0f / gamma) * 255),
            uint8_t(::powf(rgb.b / 255.0f, 1.0f / gamma) * 255),
        };
    };

    Bitmap<RGBColor> gammaCorrected = frame.map<RGBColor>(gammaCorrect);*/

    const uint8_t* pixelPointer[] = { reinterpret_cast<const uint8_t*>(frame.pixels.get()), 0 };
    const int linesizeIn[] = { int(frame.width * sizeof(RGBColor)) };

    sws_scale(swsContext, pixelPointer, linesizeIn, 0,
        frame.height, picture->data, picture->linesize);

    picture->pts = frameIndex++;

    /* encode the image */
    encode(codecContext, picture, pkt, file);
}

#endif // FFMPEG_ENABLED
