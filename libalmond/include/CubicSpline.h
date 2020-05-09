#ifndef CUBICSPLINE_H
#define CUBICSPLINE_H

#include <vector>
#include <utility>
#include <tuple>

class CubicSpline
{
    /// contains x, y and y' of each interpolation point
    std::vector<std::tuple<float, float, float>> points;
    bool useSlopes;
public:
    CubicSpline(const std::vector<std::pair<float, float>>& dataPoints, bool useSlopes);

    float interpolateAt(float x);
};

#endif // CUBICSPLINE_H
