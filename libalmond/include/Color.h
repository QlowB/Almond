#pragma once
#ifndef COLOR_H_
#define COLOR_H_

#include <cinttypes>


struct RGBColor;
struct RGBColorf;

///
/// \brief Represents a color in RGB color space with 8-bit channels
///
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


///
/// \brief Represents a color in a RGB color space with 32-bit floating
///        point numbers as channels.
///
struct RGBColorf
{
    float r, g, b;

    inline RGBColorf(void) :
        r{ 0.0f }, g{ 0.0f }, b{ 0.0f}
    {
    }


    inline RGBColorf(float r, float g, float b) :
        r{ r }, g{ g }, b{ b }
    {
    }


    RGBColorf(const RGBColor& srgb);


    ///
    /// \brief calculates the channel-wise sum of two colors
    ///
    inline RGBColorf operator + (const RGBColorf& o) const
    {
        return RGBColorf {
            r + o.r,
            g + o.g,
            b + o.b
        };
    }


    ///
    /// \brief calculates the channel-wise difference between two colors
    ///
    inline RGBColorf operator - (const RGBColorf& o) const
    {
        return RGBColorf {
            r - o.r,
            g - o.g,
            b - o.b
        };
    }


    ///
    /// \brief multiplies all channels by a factor
    ///
    inline RGBColorf operator * (float f) const
    {
        return RGBColorf {
            r * f,
            g * f,
            b * f
        };
    }


    ///
    /// \brief divides all channels by a factor
    ///
    inline RGBColorf operator / (float f) const
    {
        return this->operator * (1.0f / f);
    }
};

#endif // COLOR_H_
