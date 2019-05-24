#include "MandelUtil.h"

using mnd::MandelViewport;

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
    x += width * (1 - scale) / 2;
    y += height * (1 - scale) / 2;
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