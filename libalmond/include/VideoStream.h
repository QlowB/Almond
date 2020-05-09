#ifndef VIDEO_STREAM_H_
#define VIDEO_STREAM_H_

#define FFMPEG_ENABLED
#ifdef FFMPEG_ENABLED

#include <stdexcept>

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

namespace alm
{
    struct VideoExportException;
}

struct alm::VideoExportException :
    public std::runtime_error
{
    VideoExportException(const std::string& err);
};

class VideoStream
{
    const AVCodec* codec;
    AVCodecContext* codecContext;
    AVFormatContext* formatContext;
    AVCodecParameters* params;
    //FILE* file;
    AVFrame* picture;
    AVPacket* pkt;
    AVStream* stream;
    SwsContext* swsContext;

    int width;
    int height;

    int64_t frameIndex = 0;
public:
    VideoStream(int width, int height, const std::string& filename, int bitrate, int fps, const char* preset);
    ~VideoStream(void);

    void addFrame(const Bitmap<RGBColor>& frame);
private:
    void encode(AVFrame* frame);
};


#endif // FFMPEG_ENABLED

#endif // VIDEO_STREAM_H_
