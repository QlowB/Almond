#include "VideoStream.h"

#include <iostream>

using alm::VideoExportException;

VideoExportException::VideoExportException(const std::string& err) :
    std::runtime_error{ err }
{
}


#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free  avcodec_free_frame
#endif


VideoStream::VideoStream(int width, int height, const std::string& filename, int bitrate, int fps, const char* preset) :
    width{ width & (~1) }, height{ height & (~1) }
{
    // only needed with ffmpeg version < 4
    //avcodec_register_all();

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        throw VideoExportException{ "could not find h264 encoder" };
    }

    AVOutputFormat* oformat = av_guess_format(nullptr, filename.c_str(), nullptr);
    if (!oformat)
        oformat = av_guess_format("mp4", nullptr, nullptr);
    if (oformat == nullptr)
        throw VideoExportException{ "invalid format" };

    codecContext = avcodec_alloc_context3(codec);

    pkt = av_packet_alloc();
    if (!pkt)
        throw VideoExportException{ "could not allocate packet" };

    codecContext->bit_rate = bitrate * 1000;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = AVRational{ 1, fps };
    codecContext->framerate = AVRational{ fps, 1 };

    codecContext->gop_size = 16; /* one intra frame every 16 frames */
    codecContext->max_b_frames = 3;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    formatContext = avformat_alloc_context();
    if (!formatContext)
        throw VideoExportException{ "error allocating format context" };
    formatContext->oformat = oformat;
    formatContext->video_codec_id = oformat->video_codec;

    stream = avformat_new_stream(formatContext, codec);
    if (!stream)
        throw VideoExportException{ "error creating stream" };

    params = avcodec_parameters_alloc();
    avcodec_parameters_from_context(params, codecContext);
    stream->codecpar = params;

    /*AVCPBProperties *props;
    props = (AVCPBProperties*) av_stream_new_side_data(
        stream, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*props));
    props->buffer_size = 1024 * 1024;
    props->max_bitrate = 0;
    props->min_bitrate = 0;
    props->avg_bitrate = 0;
    props->vbv_delay = UINT64_MAX;*/

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(codecContext->priv_data, "preset", preset, 0);

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        throw VideoExportException{ "could not open codec" };
    }
    int opened = avio_open(&formatContext->pb, filename.c_str(), AVIO_FLAG_WRITE);
    if (opened < 0) {
        throw VideoExportException{ std::string("could not open file '") + filename + "'" };
    }

    if (avformat_write_header(formatContext, nullptr) < 0) {
        throw VideoExportException{ "error writing header" };
    }

    picture = av_frame_alloc();
    if (!picture)
        throw VideoExportException{ "error allocating frame" };
    picture->format = codecContext->pix_fmt;
    picture->width  = codecContext->width;
    picture->height = codecContext->height;

    int retval = av_frame_get_buffer(picture, 0);
    if (retval < 0) {
        throw VideoExportException{ "could not allocate frame data" };
    }
    if (av_frame_make_writable(picture) < 0) {
        throw VideoExportException{ "error making frame writeable" };
    }
    //av_image_alloc(picture->data, picture->linesize, width, height, codecContext->pix_fmt, 32);

    swsContext = sws_getContext(width, height,
        AV_PIX_FMT_RGB24, width, height,
        AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
    if (!swsContext)
        throw VideoExportException{ "error preparing sws context" };
}


VideoStream::~VideoStream()
{
    /* flush the encoder */
    encode(nullptr);
    av_write_trailer(this->formatContext);

    avcodec_close(codecContext);
    avio_close(formatContext->pb);
    av_frame_unref(picture);
    avcodec_parameters_free(&params);
    avcodec_free_context(&codecContext);
    av_frame_free(&picture);
    av_packet_free(&pkt);
}


void VideoStream::addFrame(const Bitmap<RGBColor>& frame)
{
    int retval = av_frame_make_writable(picture);
    if (retval < 0)
        throw VideoExportException{ "could not write to frame data" };

    /*Bitmap<RGBColor> gammaCorrected = frame.map<RGBColor>(gammaCorrect);*/

    const uint8_t* pixelPointer[] = { reinterpret_cast<const uint8_t*>(frame.pixels.get()), 0 };
    const int linesizeIn[] = { int(frame.width * sizeof(RGBColor)) };

    sws_scale(swsContext, pixelPointer, linesizeIn, 0,
        frame.height, picture->data, picture->linesize);

    picture->pts = frameIndex++;

    /* encode the image */
    encode(picture);
}


void VideoStream::encode(AVFrame* frame)
{
    int ret;

    ret = avcodec_send_frame(codecContext, frame);
    if (ret < 0) {
        throw VideoExportException{ "error encoding frame" };
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, pkt);
        //ret = avcodec_encode_video2(codecContext, pkt, picture, &gotPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            throw VideoExportException{ "error during encoding" };
        }

        //printf("encoded frame %3d\"PRId64\" (size=%5d)\n", pkt->pts, pkt->size);
        //fwrite(pkt->data, 1, pkt->size, outfile);
        //av_interleaved_write_frame(formatContext, pkt);

        av_packet_rescale_ts(pkt, AVRational{ 1, 60 }, stream->time_base);
        pkt->stream_index = stream->index;

        av_write_frame(formatContext, pkt);
        av_packet_unref(pkt);
    }
}



