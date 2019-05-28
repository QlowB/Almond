#ifndef GRADIENT_H
#define GRADIENT_H

#include <vector>
#include "Color.h"


class Gradient
{
    std::vector<std::pair<RGBColor, float>> colors;
    float max;
public:
    Gradient();

    void addColor(RGBColor c, float value);

    RGBColor get(float x) const;

private:
    static RGBColor lerpColors(RGBColor a, RGBColor b, float val);
    std::tuple<RGBColor, RGBColor, float> getNeighbors(float x) const;
};

#endif // GRADIENT_H
