#ifndef LIBALMOND_IMAGEEXPORT_H
#define LIBALMOND_IMAGEEXPORT_H

#include "Mandel.h"
#include "Gradient.h"
#include "../../libmandel/include/Mandel.h"

namespace alm
{
    struct ImageExportInfo
    {
        mnd::MandelInfo drawInfo;
        mnd::MandelGenerator* generator;
        const Gradient* gradient;
    };

    void exportPng(const std::string& name, const ImageExportInfo& iei);
}



#endif // LIBALMOND_IMAGEEXPORT_H

