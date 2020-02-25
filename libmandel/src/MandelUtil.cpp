#include "MandelUtil.h"

using mnd::MandelViewport;
using mnd::MandelInfo;


void MandelViewport::adjustAspectRatio(double nwidth, double nheight)
{
    double otherRatio = nwidth / nheight;
    if (width < height * otherRatio)
        width = height * otherRatio;
    else if (height < width / otherRatio)
        height = width / otherRatio;
}


void MandelViewport::normalize(void)
{
    if (width < 0) {
        x += width;
        width = -width;
    }
    if (height < 0) {
        y += height;
        height = -height;
    }
}


void MandelViewport::zoomCenter(float scale)
{
    /*x += width * (1 - scale) / 2;
    y += height * (1 - scale) / 2;
    width *= scale;
    height *= scale;*/
    zoom(scale, 0.5f, 0.5f);
}


void MandelViewport::zoom(float scale, float xz, float yz)
{
    mnd::Real scaleR = 1.0f - scale;
    this->x += width * scaleR * mnd::Real(xz);
    this->y += height * scaleR * mnd::Real(yz);
    width *= scale;
    height *= scale;
}


MandelViewport MandelViewport::standardView(void)
{
    return MandelViewport{
        -2.25,
        -1.5,
        3,
        3
    };
}


MandelViewport MandelViewport::centerView(void)
{
    return MandelViewport{
        -2,
        -2,
        4,
        4
    };
}


