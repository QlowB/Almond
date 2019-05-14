#include "QueueManager.h"
#include <cmath>

void MandelViewport::adjustAspectRatio(double nwidth, double nheight)
{
    double otherRatio = nwidth / nheight;
    if (width < height * otherRatio)
        width = height * otherRatio;
    else if (height < width / otherRatio)
            height = width / otherRatio;
}

MandelGenerator::~MandelGenerator(void)
{
}

Bitmap<RGBColor> MandelGenerator::generate(const MandelInfo& mandelInfo)
{
    auto converter = [](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.15f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(sin(i * 0.04f) * 127 + 127) }; };
    return generateRaw(mandelInfo).map<RGBColor>(converter);
}


QueueManager::QueueManager()
{
}


QueueManager::~QueueManager()
{
}
