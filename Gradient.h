#ifndef GRADIENT_H
#define GRADIENT_H

#include <QString>
#include <vector>
#include "Color.h"
#include <tuple>
#include <cinttypes>


class Gradient
{
    /// the colors of this gradient stored in linear RGB format
    /// so they can be easily interpolated
    std::vector<RGBColor> colors;
    float max;
    bool repeat;
public:
    Gradient(void);
    Gradient(const std::vector<std::pair<RGBColor, float>>&, bool repeat = false, int precalcSteps = 10000);

    static Gradient defaultGradient(void);

    static Gradient readXml(const QString& xml);

    /*!
     * \brief get a color at a specific position in this gradient
     * \param x the position
     * \return the color in sRGB format
     */
    RGBColor get(float x) const;
private:
    static RGBColor lerpColors(RGBColor a, RGBColor b, float val);
    std::tuple<RGBColor, RGBColor, float> getNeighbors(float x) const;
};

#endif // GRADIENT_H
