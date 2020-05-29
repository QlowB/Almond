#include "Color.h"
#include <cmath>
#include <algorithm>


RGBColor::RGBColor(const RGBColorf& rgb)
{
    float cr = std::clamp(rgb.r, 0.0f, 1.0f);
    float cg = std::clamp(rgb.g, 0.0f, 1.0f);
    float cb = std::clamp(rgb.b, 0.0f, 1.0f);
    r = uint8_t(cr * cr * 255.0f);
    g = uint8_t(cg * cg * 255.0f);
    b = uint8_t(cb * cb * 255.0f);
}


RGBColorf::RGBColorf(const RGBColor& srgb)
{
    r = ::sqrtf(srgb.r / 255.0f);
    g = ::sqrtf(srgb.g / 255.0f);
    b = ::sqrtf(srgb.b / 255.0f);
}
