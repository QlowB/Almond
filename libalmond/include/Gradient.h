#ifndef GRADIENT_H
#define GRADIENT_H

#include <vector>
#include <string>
#include "Color.h"
#include <tuple>
#include <cinttypes>


class Gradient
{
    std::vector<std::pair<RGBColor, float>> points;
    /// the colors of this gradient stored in linear RGB format
    /// so they can be easily interpolated
    std::vector<RGBColorf> colors;
    float max;
    bool repeat;
public:
    Gradient(void);
    Gradient(std::vector<std::pair<RGBColor, float>> colors, bool repeat = false, int precalcSteps = -1);
    Gradient(std::vector<std::pair<RGBColor, float>> colors, float maxValue, bool repeat = false, int precalcSteps = -1);

    const std::vector<std::pair<RGBColor, float>>& getPoints(void) const;

    static Gradient defaultGradient(void);

    static Gradient fromXml(const std::string& xml);

    ///
    /// \brief get the maximum value this gradient accepts
    ///
    /// If \link Gradient::get(float) is called with a value
    /// greater than the one returned by this function, the
    /// value will either get clamped or wrapped around.
    ///
    float getMax(void) const;

    ///
    /// \brief get a color at a specific position in this gradient
    /// \param x the position
    /// \return the color in sRGB format
    ///
    RGBColor get(float x) const;
private:
    static RGBColorf lerpColors(RGBColorf a, RGBColorf b, float val);
    static RGBColor lerpColors(RGBColor a, RGBColor b, float val);
    std::tuple<RGBColor, RGBColor, float> getNeighbors(float x) const;
};

#endif // GRADIENT_H
