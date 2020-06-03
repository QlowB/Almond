#include "CubicSpline.h"
#include <cmath>

CubicSpline::CubicSpline(const std::vector<std::pair<float, float> >& dataPoints,
                         bool useSlopes, bool minSlopes) :
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

        float slope;
        if (minSlopes) {
            if (fabs(s1) > fabs(s2))
                slope = s2;
            else
                slope = s1;
        } else
            slope = (s1 + s2) / 2;
        points.push_back({ dp2.first, dp2.second, slope });
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

    if (points.empty()) {
        return 0.0f;
    }

    if(std::get<0>(points[0]) > x) {
        return std::get<1>(points[0]);
    }

    for (auto it = points.begin(); it != points.end() && (it + 1) != points.end(); ++it) {
        auto& left = *it;
        auto& right = *(it + 1);
        float xleft = std::get<0>(left);
        float xright = std::get<0>(right);
        if (xleft <= x && xright >= x) {
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


ColorSpline::ColorSpline(const std::vector<std::pair<float, RGBColorf> >& dataPoints,
                         bool useSlopes, bool minSlopes) :
    useSlopes{ useSlopes }
{
    if (dataPoints.size() < 1) {
        points.emplace(0.0f, std::make_pair(RGBColorf{ 0, 0, 0 }, RGBColorf{ 0, 0, 0 }));
        return;
    }
    if (dataPoints.size() == 1) {
        points.emplace(dataPoints[0].first, std::make_pair(dataPoints[0].second, RGBColorf{ 0, 0, 0 }));
        return;
    }
    
    const auto& dpf = dataPoints[0];
    const auto& dps = dataPoints[1];
    points.emplace(dpf.first, std::make_pair(dpf.second,
                       (dps.second - dpf.second) / (dps.first - dpf.first)));
    for (size_t i = 1; i < dataPoints.size() - 1; i++) {

        const auto& dp1 = dataPoints[i - 1];
        const auto& dp2 = dataPoints[i];
        const auto& dp3 = dataPoints[i + 1];

        float w1 = dp2.first - dp1.first;
        float w2 = dp3.first - dp2.first;
        RGBColorf h1 = dp2.second - dp1.second;
        RGBColorf h2 = dp3.second - dp2.second;

        RGBColorf s1 = h1 / w1;
        RGBColorf s2 = h2 / w2;

        RGBColorf slope;
        if (minSlopes) {
            auto minAbs = [] (float a, float b) { return fabs(a) > fabs(b) ? b : a; };
            slope.r = minAbs(s1.r, s2.r);
        } else
            slope = (s1 + s2) / 2;
        points.emplace(dp2.first, std::make_pair(dp2.second, slope));
    }

    const auto& dpSecondLast = dataPoints[dataPoints.size() - 2];
    const auto& dpLast = dataPoints[dataPoints.size() - 1];
    points.emplace(dpLast.first, std::make_pair(dpLast.second,
                       (dpSecondLast.second - dpLast.second) /
                       (dpSecondLast.first - dpLast.first)));
}


RGBColorf ColorSpline::interpolateAt(float x) const
{
    const static auto h00 = [] (float t) { return (1 + 2 * t) * (1 - t) * (1 - t); };
    const static auto h01 = [] (float t) { return t * t * (3 - 2 * t); };
    const static auto h10 = [] (float t) { return t * (1 - t) * (1 - t); };
    const static auto h11 = [] (float t) { return t * t * (t - 1); };

    if (points.empty()) {
        return RGBColorf{ 0, 0, 0 };
    }


    auto firstAfter = points.upper_bound(x);
    if (firstAfter == points.begin()) {
        return firstAfter->second.first;
    }
    auto lastBefore = firstAfter; lastBefore--;
    if (firstAfter == points.end()) {
        return lastBefore->second.first;
    }

    float xLeft = lastBefore->first;
    float xRight = firstAfter->first;

    RGBColorf yLeft = lastBefore->second.first;
    RGBColorf yRight = firstAfter->second.first;
    RGBColorf slopeLeft = lastBefore->second.second;
    RGBColorf slopeRight = firstAfter->second.second;

    float t = (x - xLeft) / (xRight - xLeft);

    RGBColorf interpolated = yLeft * h00(t) + yRight * h01(t);
    if (useSlopes) {
        interpolated = interpolated + slopeLeft * h10(t) * (xRight - xLeft);
        interpolated = interpolated + slopeRight * h11(t) * (xRight - xLeft);
    }

    return interpolated;

    /*
    for (auto it = points.begin(); it != points.end() && (it + 1) != points.end(); ++it) {
        auto& left = *it;
        auto& right = *(it + 1);
        float xleft = std::get<0>(left);
        float xright = std::get<0>(right);
        if (xleft <= x && xright >= x) {
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
    return std::get<1>(points[points.size() - 1]);*/
}
