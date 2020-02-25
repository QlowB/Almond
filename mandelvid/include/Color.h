#pragma once
#ifndef COLOR_H_
#define COLOR_H_

#include <cinttypes>


struct RGBColor;
struct RGBColorf;

/*!
 * \brief Represents a color in the sRGB color space with 8-bit channels
 */
struct RGBColor
{
    uint8_t r, g, b;

    inline RGBColor(void) :
        r{ 0 }, g{ 0 }, b{ 0 }
    {
    }

    inline RGBColor(uint8_t r, uint8_t g, uint8_t b) :
        r{ r }, g{ g }, b{ b }
    {
    }

    RGBColor(const RGBColorf& rgb);
};


/*!
 * \brief Represents a color in a linear RGB color space with 32-bit floating
 *        point numbers as channels.
 */
struct RGBColorf
{
    float r, g, b;

    inline RGBColorf(float r, float g, float b) :
        r{ r }, g{ g }, b{ b }
    {
    }

    RGBColorf(const RGBColor& srgb);
};

#endif // COLOR_H_
