#include "Color.h"
#include <cmath>
#include <algorithm>

const static float cGamma = 1.0f;

RGBColor::RGBColor(const RGBColorf& rgb)
{
    float cr = std::clamp(rgb.r, 0.0f, 1.0f);
    float cg = std::clamp(rgb.g, 0.0f, 1.0f);
    float cb = std::clamp(rgb.b, 0.0f, 1.0f);
    
    const float invGamma = 1.0f / cGamma;
    r = uint8_t(::powf(cr, invGamma) * 255.0f);
    g = uint8_t(::powf(cg, invGamma) * 255.0f);
    b = uint8_t(::powf(cb, invGamma) * 255.0f);
}


RGBColorf::RGBColorf(const RGBColor& srgb)
{
    r = ::powf(srgb.r / 255.0f, cGamma);
    g = ::powf(srgb.g / 255.0f, cGamma);
    b = ::powf(srgb.b / 255.0f, cGamma);
}
