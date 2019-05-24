#pragma once
#define FFMPEG_ENABLED
#ifdef FFMPEG_ENABLED


#ifndef VIDEO_STREAM_H_
#define VIDEO_STREAM_H_

#include <string>
#include "Bitmap.h"

extern "C" {
#   include <libavformat/avformat.h>
#   include <libavformat/avio.h>
#   include <libavcodec/avcodec.h>
#   include <libavformat/avformat.h>
#   include <libavutil/imgutils.h>
#   include <libavutil/opt.h>
#   include <libswscale/swscale.h>
}

class VideoStream
{
    const AVCodec* codec;
    AVCodecContext* codecContext;
    FILE* file;
    AVFrame* picture;
    AVPacket* pkt;
    SwsContext* swsContext;
    static const uint8_t endcode[];

    size_t width;
    size_t height;

    uint64_t frameIndex = 0;
public:
    VideoStream(::size_t width, ::size_t height, const std::string& filename);
    ~VideoStream(void);

    void addFrame(const Bitmap<RGBColor>& frame);
};

#endif // VIDEO_STREAM_H_

#endif // FFMPEG_ENABLED
