#include "CubicSpline.h"

CubicSpline::CubicSpline(const std::vector<std::pair<float, float> >& dataPoints, bool useSlopes) :
    useSlopes{ useSlopes }
{
    if (dataPoints.size() < 2) {
        return;
    }

    points.push_back({ dataPoints[0].first, dataPoints[0].second,
                       (dataPoints[1].second - dataPoints[0].second) /
                       (dataPoints[1].first - dataPoints[0].first) });
    for (size_t i = 1; i < dataPoints.size() - 1; i++) {

        auto& dp1 = dataPoints[i - 1];
        auto& dp2 = dataPoints[i];
        auto& dp3 = dataPoints[i + 1];

        float w1 = dp2.first - dp1.first;
        float w2 = dp3.first - dp2.first;
        float h1 = dp2.second - dp1.second;
        float h2 = dp3.second - dp2.second;

        float s1 = h1 / w1;
        float s2 = h2 / w2;

        float avgSlope = (s1 + s2) / 2;
        points.push_back({ dp2.first, dp2.second, avgSlope });
    }
    points.push_back({ dataPoints[dataPoints.size() - 1].first, dataPoints[dataPoints.size() - 1].second,
                       (dataPoints[dataPoints.size() - 2].second - dataPoints[dataPoints.size() - 1].second) /
                       (dataPoints[dataPoints.size() - 2].first - dataPoints[dataPoints.size() - 1].first) });
}


float CubicSpline::interpolateAt(float x)
{
    const static auto h00 = [] (float t) { return (1 + 2 * t) * (1 - t) * (1 - t); };
    const static auto h01 = [] (float t) { return t * t * (3 - 2 * t); };
    const static auto h10 = [] (float t) { return t * (1 - t) * (1 - t); };
    const static auto h11 = [] (float t) { return t * t * (t - 1); };
    for (auto it = points.begin(); it != points.end() && (it + 1) != points.end(); ++it) {
        auto& left = *it;
        auto& right = *(it + 1);
        float xleft = std::get<0>(left);
        float xright = std::get<0>(right);
        if (xleft < x && xright >= x) {
            float w = (xright - xleft);
            float t = (x - xleft) / w;
            float yleft = std::get<1>(left);
            float yright = std::get<1>(right);
            float sleft = std::get<2>(left);
            float sright = std::get<2>(right);

            float inter = h00(t) * yleft +
                          h01(t) * yright;

            if (useSlopes)
                inter += h10(t) * w * sleft +
                         h11(t) * w * sright;

            return inter;
        }
    }
    return std::get<1>(points[points.size() - 1]);
}
