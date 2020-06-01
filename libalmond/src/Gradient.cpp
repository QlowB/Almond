#include "Gradient.h"

#include "CubicSpline.h"

#include "../tinyxml2/tinyxml2.h"
#include "XmlException.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <functional>


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

    points = colors;
    max = colors.at(colors.size() - 1).second;

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

    points = colors;
    max = maxVal;

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


Gradient Gradient::fromXml(const std::string& xml)
{
    using tinyxml2::XMLDocument;
    using tinyxml2::XMLError;
    using tinyxml2::XMLNode;
    using tinyxml2::XMLElement;
    using tinyxml2::XML_SUCCESS;

    XMLDocument xmlDoc;
    XMLError err = xmlDoc.Parse(xml.c_str());
    if (err != XML_SUCCESS)
        throw alm::XmlException{ "error parsing gradient xml" };
    XMLNode* rootNode = xmlDoc.FirstChild();
    if (rootNode == nullptr)
        throw alm::XmlException{ "invalid root node" };

    bool repeat = false;
    bool hasMax = false;
    float maxVal = 0.0f;
    if (auto* re = dynamic_cast<XMLElement*>(rootNode)) {
        repeat = re->BoolAttribute("repeat", false);
        XMLError e = re->QueryFloatAttribute("max", &maxVal);
        if (e == XML_SUCCESS)
            hasMax = true;
    }

    std::vector<std::pair<RGBColor, float>> points;

    XMLElement* colorNode = rootNode->FirstChildElement("color");
    while(colorNode != nullptr) {
        int r = colorNode->IntAttribute("r");
        int g = colorNode->IntAttribute("g");
        int b = colorNode->IntAttribute("b");
        float p = colorNode->FloatAttribute("p");
        
        uint8_t cr = uint8_t(std::clamp(r, 0, 255));
        uint8_t cg = uint8_t(std::clamp(g, 0, 255));
        uint8_t cb = uint8_t(std::clamp(b, 0, 255));
        points.push_back(std::make_pair(RGBColor{ cr, cg, cb }, p));

        colorNode = colorNode->NextSiblingElement("color");
    }
    if (hasMax)
        return Gradient{ std::move(points), maxVal, repeat };
    else
        return Gradient{ std::move(points), repeat };
}


std::string Gradient::toXml(void) const
{
    std::stringstream buf;

    buf << "<gradient max=\"" << max << "\" repeat=\"" << (repeat ? "true" : "false") << "\">" << std::endl;
    for (const auto&[color, val] : points) {
        buf << "    <color " <<
               "r=\"" << int(color.r) <<
               "\" g=\"" << int(color.g) <<
               "\" b=\"" << int(color.b) <<
               "\" p=\"" << val << "\" />" << std::endl;
    }
    buf << "</gradient>" << std::endl;
    return buf.str();
}


float Gradient::getMax(void) const
{
    return this->max;
}


RGBColor Gradient::get(float x) const
{
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
