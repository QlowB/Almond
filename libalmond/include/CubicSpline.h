#ifndef CUBICSPLINE_H
#define CUBICSPLINE_H

#include <vector>
#include <utility>
#include <tuple>
#include <map>
#include <Color.h>

class CubicSpline
{
    /// contains x, y and y' of each interpolation point
    std::vector<std::tuple<float, float, float>> points;
    bool useSlopes;
public:
    CubicSpline(const std::vector<std::pair<float, float>>& dataPoints,
                bool useSlopes, bool minSlopes);

    float interpolateAt(float x);
};


class ColorSpline
{
    ///
    /// \brief at each position, stores the color value and the channel-wise
    ///        slopes of the spline.
    ///
    std::map<float, std::pair<RGBColorf, RGBColorf>> points;
    bool useSlopes;
public:
    ColorSpline(void) = default;
    ColorSpline(const std::vector<std::pair<float, RGBColorf>>& dataPoints,
                bool useSlopes, bool minSlopes);

    RGBColorf interpolateAt(float x) const;
};

#endif // CUBICSPLINE_H
