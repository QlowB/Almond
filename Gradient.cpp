#include "Gradient.h"

#include <cmath>
#include <algorithm>
#include <QtXml/QDomDocument>
#include <QFile>


Gradient::Gradient(void) :
    max{ 1.0 }
{
}


Gradient::Gradient(const std::vector<std::pair<RGBColor, float>>& colors, bool repeat, int precalcSteps) :
    repeat{ repeat }
{
    if(colors.empty())
        return;
    /*std::sort(colors.begin(), colors.end(),
              [] (const std::pair<RGBColor, float>& a, const std::pair<RGBColor, float>& b) {
        return a.second < b.second;
    });*/

    max = colors.at(colors.size() - 1).second;
    for (int i = 0; i < precalcSteps; i++) {
        float position = i * max / precalcSteps;
        RGBColor left = RGBColor{ 0, 0, 0 };
        RGBColor right = RGBColor{ 0, 0, 0 };
        float lerp = 0.0f;
        RGBColor atPosition = RGBColor{ 0, 0, 0 };
        // slow, but not in any critical path
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            if (it->second > position) {
                if (it == colors.begin()) {
                    atPosition = it->first;
                    break;
                }
                else {
                    float lerp = (position - (it - 1)->second) / (it->second - (it - 1)->second);
                    atPosition = lerpColors((it - 1)->first, it->first, lerp);
                    break;
                }
            }
        }
        this->colors.push_back(atPosition);
    }
}


Gradient Gradient::defaultGradient(void)
{
    QFile res(":/gradients/default");
    res.open(QIODevice::ReadOnly);
    QString str = QString::fromUtf8(res.readAll());
    return readXml(str);
}

Gradient Gradient::readXml(const QString& xml)
{
    QDomDocument xsr;
    xsr.setContent(xml);
    auto elem = xsr.documentElement();
    auto repeatAttr = elem.attributeNode("repeat");
    bool repeat = !repeatAttr.isNull() && repeatAttr.value().toLower() == "true";
    auto colors = xsr.elementsByTagName("color");
    std::vector<std::pair<RGBColor, float>> colorArr;
    for (int i = 0; i < colors.length(); ++i) {
        auto child = colors.item(i).toElement();
        uint8_t r = child.attributeNode("r").value().toInt();
        uint8_t g = child.attributeNode("g").value().toInt();
        uint8_t b = child.attributeNode("b").value().toInt();
        float p = child.attributeNode("p").value().toInt();

        printf("rgb (%s): %d, %d, %d\n", child.text().toUtf8().data(), r, g, b);
        colorArr.push_back({ { r, g, b }, p });
    }

    return Gradient(colorArr, repeat);
}


RGBColor Gradient::get(float x) const
{
    if (colors.empty())
        return RGBColor();
    /*const auto [left, right, lerp] = getNeighbors(x);
    RGBColor lerped = lerpColors(left, right, lerp);
    return lerped;*/

    if (x > this->max) {
        if (repeat)
            x = ::fmodf(x, this->max);
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

    if (lerp < 1e-5) {
        return colors.at(left);
    }
    else {
        return lerpColors(colors.at(left), colors.at(right), lerp);
    }
}


RGBColor Gradient::lerpColors(RGBColor a, RGBColor b, float val)
{
    auto mklin = [] (double x) {
        return x * x;//::pow(x, 2.4);
    };
    auto unlin = [] (double x) {
        return ::sqrt(x);// ::pow(x, 1.0 / 2.4);
    };

    return RGBColor{
        uint8_t(unlin(mklin(b.r) * val + mklin(a.r) * (1 - val))),
        uint8_t(unlin(mklin(b.g) * val + mklin(a.g) * (1 - val))),
        uint8_t(unlin(mklin(b.b) * val + mklin(a.b) * (1 - val)))
    };
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
