#include "Gradient.h"

#include "CubicSpline.h"

#include "XmlException.h"

#include <cmath>
#include <algorithm>
#include <functional>

using alm::Gradient;


Gradient::Gradient(void) :
    max{ 1.0 }
{
}


Gradient::Gradient(std::vector<std::pair<RGBColor, float>> colors, bool repeat, int precalcSteps) :
    repeat{ repeat }
{
    if(colors.empty() || colors.size() < 2)
        return;
    std::sort(colors.begin(), colors.end(),
        [] (const auto& a, const auto& b) {
            return a.second < b.second;
        });

    for (auto& [col, at] : colors) {
        pointMap[at] = col;
    }

    points = colors;
    max = colors.at(colors.size() - 1).second;

    std::vector<std::pair<float, RGBColorf>> fs;
    for (const auto& [col, pos] : points) {
        fs.push_back(std::make_pair(pos, col));
    }
    colorSpline = ColorSpline{ fs, false, false };
    
    return;
    std::vector<std::pair<RGBColorf, float>> linearColors;
    std::transform(colors.begin(), colors.end(), std::back_inserter(linearColors),
                   [] (auto c) { return c; });

    std::vector<std::pair<float, float>> rs;
    std::vector<std::pair<float, float>> gs;
    std::vector<std::pair<float, float>> bs;

    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(rs),
                   [] (auto p) { return std::pair{ p.second, p.first.r }; });
    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(gs),
                   [] (auto p) { return std::pair{ p.second, p.first.g }; });
    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(bs),
                   [] (auto p) { return std::pair{ p.second, p.first.b }; });

    CubicSpline rsp(rs, false, true);
    CubicSpline gsp(gs, false, true);
    CubicSpline bsp(bs, false, true);

    if(precalcSteps <= 0) {
        precalcSteps = int(max * 7) + 10;
    }
    if (precalcSteps > 12000) {
        precalcSteps = 12000;
    }

    for (int i = 0; i < precalcSteps; i++) {
        float position = i * max / precalcSteps;
        RGBColorf at = {
            rsp.interpolateAt(position),
            gsp.interpolateAt(position),
            bsp.interpolateAt(position)
        };
        this->colors.push_back(at);
    }
}


Gradient::Gradient(std::vector<std::pair<RGBColor, float>> colors, float maxVal, bool repeat, int precalcSteps) :
    repeat{ repeat }
{
    if(colors.empty() || colors.size() < 2)
        return;
    std::sort(colors.begin(), colors.end(),
        [] (const auto& a, const auto& b) {
            return a.second < b.second;
        });

    for (auto& [col, at] : colors) {
        pointMap[at] = col;
    }

    points = colors;
    max = maxVal;
    

    std::vector<std::pair<float, RGBColorf>> fs;
    for (const auto& [col, pos] : points) {
        fs.push_back(std::make_pair(pos, col));
    }
    colorSpline = ColorSpline{ fs, false, false };
    return;
    std::vector<std::pair<RGBColorf, float>> linearColors;
    std::transform(colors.begin(), colors.end(), std::back_inserter(linearColors),
                   [] (auto c) { return c; });

    std::vector<std::pair<float, float>> rs;
    std::vector<std::pair<float, float>> gs;
    std::vector<std::pair<float, float>> bs;

    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(rs),
                   [] (auto p) { return std::pair{ p.second, p.first.r }; });
    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(gs),
                   [] (auto p) { return std::pair{ p.second, p.first.g }; });
    std::transform(linearColors.begin(), linearColors.end(), std::back_inserter(bs),
                   [] (auto p) { return std::pair{ p.second, p.first.b }; });

    CubicSpline rsp(rs, false, true);
    CubicSpline gsp(gs, false, true);
    CubicSpline bsp(bs, false, true);

    if(precalcSteps <= 0) {
        precalcSteps = int(max * 7) + 10;
    }
    if (precalcSteps > 12000) {
        precalcSteps = 12000;
    }

    for (int i = 0; i < precalcSteps; i++) {
        float position = i * max / precalcSteps;
        RGBColorf at = {
            rsp.interpolateAt(position),
            gsp.interpolateAt(position),
            bsp.interpolateAt(position)
        };
        this->colors.push_back(at);
    }
}


const std::vector<std::pair<RGBColor, float>>& Gradient::getPoints(void) const
{
    return points;
}


Gradient Gradient::defaultGradient(void)
{
    return Gradient({
        { RGBColor{ 0, 0, 0 }, 0.0f },
        { RGBColor{ 0, 255, 255 }, 30.0f },
        { RGBColor{ 50, 100, 170 }, 60.0f },
        { RGBColor{ 180, 140, 20 }, 90.0f },
        { RGBColor{ 255, 255, 0 }, 120.0f },
        { RGBColor{ 143, 67, 0 }, 150.0f },
        { RGBColor{ 255, 255, 255 }, 180.0f },
        { RGBColor{ 20, 30, 180 }, 210.0f },
        { RGBColor{ 20, 190, 30 }, 240.0f },
        { RGBColor{ 120, 240, 120 }, 270.0f },
        { RGBColor{ 0, 0, 0 }, 300.0f },
    }, true);
}


bool Gradient::isRepeat(void) const
{
    return repeat;
}


float Gradient::getMax(void) const
{
    return this->max;
}


RGBColor Gradient::get(float x) const
{
    return interpolate(x);
    if (colors.empty() || std::isnan(x) || std::isinf(x))
        return RGBColor();
    /*const auto [left, right, lerp] = getNeighbors(x);
    RGBColor lerped = lerpColors(left, right, lerp);
    return lerped;*/

    if (x < 0)
        return colors[0];
    if (x > this->max) {
        if (repeat)
            x = ::fmodf(x, this->max);
        else
            x = this->max;
    }
    float pos = x * colors.size() / max;
    if (pos < 0) {
        pos = 0;
    }
    if (pos > colors.size() - 1) {
        pos = colors.size() - 1;
    }

    int left = int(pos);
    int right = int(pos + 1);
    float lerp = pos - left;

    if (lerp < 1e-5f) {
        return colors[left];
    }
    else {
        return lerpColors(colors[left], colors[right], lerp);
    }
}


RGBColor Gradient::interpolate(float x) const
{
    return colorSpline.interpolateAt(x);
    
    if (pointMap.empty()) {
        return RGBColor{ 0, 0, 0 };
    }

    if (x < 0)
        return pointMap.begin()->second;
    if (x > this->max) {
        if (repeat)
            x = ::fmodf(x, this->max);
        else
            return (pointMap.rbegin())->second;
    }

    auto firstLess = pointMap.lower_bound(x);

    if (firstLess == pointMap.begin())
        return firstLess->second;
    if (firstLess == pointMap.end())
        return (pointMap.rbegin())->second;

    auto oneBefore = firstLess; oneBefore--;
    float lo = oneBefore->first;
    float hi = firstLess->first;
    RGBColor loCol = oneBefore->second;
    RGBColor hiCol = firstLess->second;

    return lerpColors(loCol, hiCol, (x - lo) / (hi - lo));
}


RGBColorf Gradient::lerpColors(RGBColorf a, RGBColorf b, float val)
{
    return RGBColorf {
        b.r * val + a.r * (1 - val),
        b.g * val + a.g * (1 - val),
        b.b * val + a.b * (1 - val)
    };
}


RGBColor Gradient::lerpColors(RGBColor a, RGBColor b, float val)
{
    return RGBColor{ lerpColors(RGBColorf{ a }, RGBColorf{ b }, val) };
}


/*std::tuple<RGBColor, RGBColor, float> Gradient::getNeighbors(float x) const
{
    for (auto it = colors.begin(); it != colors.end(); ++it) {
        if (it->second > x) {
            if (it == colors.begin()) {
                return { it->first, it->first, 0 };
            }
            else {
                float lerp = (x - (it - 1)->second) / (it->second - (it - 1)->second);
                return { (it - 1)->first, it->first, lerp };
            }
        }
    }
    return { (colors.end() - 1)->first, (colors.end() - 1)->first, 0 };
}*/
