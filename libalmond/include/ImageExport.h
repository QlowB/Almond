#ifndef LIBALMOND_IMAGEEXPORT_H
#define LIBALMOND_IMAGEEXPORT_H

#include "Mandel.h"
#include "Gradient.h"
#include <functional>

namespace alm
{
    struct ImageExportInfo
    {
        mnd::MandelInfo drawInfo;
        mnd::MandelGenerator* generator;
        Gradient gradient;
        std::string path;
    };

    /**
     * \brief generates and saves a fractal image in png format.
     * 
     * \param iei               info to generate the image
     * \param progressCallback  optional function that is called to
     *                          report progress; the float parameter
     *                          contains a value from 0 to 100
     */
    void exportPng(const ImageExportInfo& iei,
        std::function<void(float)> progressCallback = [](float){});
}



#endif // LIBALMOND_IMAGEEXPORT_H

