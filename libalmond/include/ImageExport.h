#ifndef LIBALMOND_IMAGEEXPORT_H
#define LIBALMOND_IMAGEEXPORT_H

#include "Mandel.h"
#include "Gradient.h"
#include <functional>
#include <vector>
#include <stdexcept>

namespace alm
{
    enum class ImageFormat
    {
        BMP,
        PNG,
        JPEG
    };
    bool supportsImageFormat(ImageFormat imgf);

    struct ImageOptions
    {
        int jpegQuality = 80;
    };

    struct ImageExportInfo
    {
        mnd::MandelInfo drawInfo;
        mnd::MandelGenerator* generator;
        Gradient gradient;
        ImageOptions options;
        std::string path;
    };

    struct ImageExportException :
        std::runtime_error
    {
        ImageExportException(const std::string& err);
    };

    ///
    /// \brief generates and saves a fractal image. The format
    ///        will be guessed by the file extension
    /// 
    /// \param iei               info to generate the image
    /// \param progressCallback  optional function that is called to
    ///                          report progress; the float parameter
    ///                          contains a value from 0 to 100
    /// 
    void exportImage(const ImageExportInfo& iei,
        std::function<void(float)> progressCallback = [](float){},
        std::function<bool(void)> cancelCallback = [](void){ return false; }
        );
}



#endif // LIBALMOND_IMAGEEXPORT_H

