#include "Gradient.h"

#include <cmath>
#include <algorithm>

/*
Gradient::Gradient()
{
    //addColor(RGBColor{255, 0, 0}, -100);
    addColor(RGBColor{0, 0, 0}, 0);
    addColor(RGBColor{ 250, 70, 24 }, 40);
    addColor(RGBColor{ 200, 230, 30 }, 104);
    addColor(RGBColor{ 70, 223, 30 }, 190);
    addColor(RGBColor{ 14, 20, 150 }, 295);
    addColor(RGBColor{ 36, 155, 169 }, 418);
    addColor(RGBColor{ 233, 33, 79 }, 558);
    addColor(RGBColor{ 254, 169, 63 }, 714);
    addColor(RGBColor{ 227, 93, 201 }, 885);
    addColor(RGBColor{ 188, 24, 161 }, 1071);
    addColor(RGBColor{ 45, 225, 44 }, 1271);
    addColor(RGBColor{ 52, 58, 250 }, 1485);
    addColor(RGBColor{ 87, 93, 241 }, 1712);
    addColor(RGBColor{ 6, 37, 208 }, 1952);
    addColor(RGBColor{ 193, 164, 162 }, 2205);
    addColor(RGBColor{ 49, 8, 90 }, 2471);
    addColor(RGBColor{ 124, 149, 120 }, 2749);
    addColor(RGBColor{ 56, 69, 6 }, 3039);
    addColor(RGBColor{ 231, 177, 252 }, 3341);
    addColor(RGBColor{ 36, 76, 95 }, 3655);
    addColor(RGBColor{ 1, 99, 184 }, 3980);
    addColor(RGBColor{ 74, 223, 199 }, 4316);
    addColor(RGBColor{ 249, 125, 31 }, 4664);
    addColor(RGBColor{ 28, 93, 214 }, 5023);
    addColor(RGBColor{ 232, 117, 145 }, 5393);
    addColor(RGBColor{ 208, 158, 49 }, 5773);
    addColor(RGBColor{ 218, 109, 177 }, 6164);
    addColor(RGBColor{ 139, 83, 177 }, 6565);
    addColor(RGBColor{ 16, 36, 59 }, 6977);
    addColor(RGBColor{ 194, 157, 26 }, 7399);
    addColor(RGBColor{ 77, 236, 12 }, 7831);
    addColor(RGBColor{ 124, 244, 151 }, 8273);
    addColor(RGBColor{ 128, 195, 162 }, 8725);
    addColor(RGBColor{ 66, 36, 194 }, 9187);
    addColor(RGBColor{ 81, 151, 185 }, 9659);
    addColor(RGBColor{ 173, 75, 175 }, 10140);
    addColor(RGBColor{ 43, 182, 52 }, 10631);
    addColor(RGBColor{ 14, 242, 141 }, 11131);
    addColor(RGBColor{ 156, 203, 87 }, 11641);
    addColor(RGBColor{ 147, 89, 150 }, 12160);
    addColor(RGBColor{ 213, 199, 183 }, 12689);
    addColor(RGBColor{ 186, 255, 52 }, 13227);
    addColor(RGBColor{ 28, 158, 154 }, 13774);
    addColor(RGBColor{ 5, 5, 116 }, 14330);
    addColor(RGBColor{ 126, 123, 232 }, 14895);
    addColor(RGBColor{ 43, 162, 251 }, 15469);
    addColor(RGBColor{ 198, 143, 125 }, 16052);
    addColor(RGBColor{ 201, 157, 178 }, 16644);
    addColor(RGBColor{ 213, 151, 189 }, 17245);
    addColor(RGBColor{ 188, 117, 169 }, 17854);
    addColor(RGBColor{ 156, 189, 249 }, 18472);
    addColor(RGBColor{ 62, 23, 33 }, 19099);
    addColor(RGBColor{ 167, 205, 74 }, 19734);
    addColor(RGBColor{ 161, 181, 210 }, 20378);
    addColor(RGBColor{ 179, 167, 215 }, 21030);
    addColor(RGBColor{ 204, 102, 126 }, 21691);
    addColor(RGBColor{ 123, 49, 127 }, 22360);
    addColor(RGBColor{ 178, 48, 136 }, 23037);
    addColor(RGBColor{ 108, 112, 99 }, 23723);
    addColor(RGBColor{ 250, 152, 78 }, 24417);
    addColor(RGBColor{ 79, 167, 196 }, 25119);
    addColor(RGBColor{ 149, 167, 8 }, 25829);
    addColor(RGBColor{ 196, 29, 159 }, 26548);
    addColor(RGBColor{ 128, 26, 20 }, 27275);
    addColor(RGBColor{ 69, 49, 66 }, 28010);
    addColor(RGBColor{ 12, 42, 198 }, 28753);
    addColor(RGBColor{ 61, 62, 36 }, 29504);
    addColor(RGBColor{ 27, 94, 114 }, 30263);
    addColor(RGBColor{ 54, 218, 7 }, 31030);
    addColor(RGBColor{ 105, 89, 170 }, 31804);
    addColor(RGBColor{ 100, 110, 2 }, 32586);
    addColor(RGBColor{ 208, 198, 148 }, 33376);
    addColor(RGBColor{ 80, 208, 131 }, 34174);
    addColor(RGBColor{ 176, 89, 59 }, 34980);
    addColor(RGBColor{ 255, 64, 243 }, 35793);
    addColor(RGBColor{ 39, 226, 232 }, 36614);
    addColor(RGBColor{ 154, 100, 238 }, 37443);
    addColor(RGBColor{ 53, 103, 192 }, 38279);
    addColor(RGBColor{ 187, 41, 136 }, 39123);
    addColor(RGBColor{ 33, 84, 227 }, 39974);
    addColor(RGBColor{ 71, 167, 211 }, 40833);
    addColor(RGBColor{ 55, 191, 255 }, 41699);
    addColor(RGBColor{ 60, 165, 201 }, 42573);
    addColor(RGBColor{ 231, 206, 192 }, 43454);
    addColor(RGBColor{ 233, 224, 197 }, 44343);
    addColor(RGBColor{ 255, 129, 13 }, 45239);
    addColor(RGBColor{ 131, 222, 95 }, 46143);
    addColor(RGBColor{ 155, 249, 72 }, 47054);
    addColor(RGBColor{ 248, 129, 30 }, 47972);
    addColor(RGBColor{ 48, 239, 206 }, 48898);
    addColor(RGBColor{ 176, 224, 64 }, 49831);
    addColor(RGBColor{ 155, 12, 162 }, 50771);
    addColor(RGBColor{ 6, 144, 149 }, 51718);
    addColor(RGBColor{ 231, 208, 16 }, 52672);
    addColor(RGBColor{ 190, 66, 231 }, 53634);
    addColor(RGBColor{ 19, 17, 253 }, 54603);
    addColor(RGBColor{ 4, 34, 60 }, 55579);
    addColor(RGBColor{ 101, 23, 88 }, 56562);
    addColor(RGBColor{ 9, 191, 235 }, 57552);


    max = 10000;
    //addColor(RGBColor{255, 0, 255}, 1000000000.0f);
}
*/

Gradient::Gradient(void) :
    max{ 1.0 }
{
}


Gradient::Gradient(const std::vector<std::pair<RGBColor, float>>& colors, int precalcSteps)
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
    std::vector<std::pair<RGBColor, float>> colors = {
        { RGBColor{0, 0, 0}, 0 },
        { RGBColor{ 250, 70, 24 }, 40 },
        { RGBColor{ 200, 230, 30 }, 80 },
        { RGBColor{ 70, 223, 30 }, 105 },
        { RGBColor{ 40, 190, 240 }, 140 },
        { RGBColor{ 120, 60, 160 }, 230 },
        { RGBColor{ 14, 20, 180 }, 300 },
        { RGBColor{ 240, 240, 240 }, 418 },
        { RGBColor{ 243, 20, 20 }, 558 },
        { RGBColor{ 254, 169, 63 }, 714 },
        { RGBColor{ 8, 50, 8 }, 885 },
        { RGBColor{ 188, 5, 161 }, 1071 },
        { RGBColor{ 45, 225, 44 }, 1271 },
        { RGBColor{ 52, 58, 250 }, 1485 },
        { RGBColor{ 87, 93, 241 }, 1712 },
        { RGBColor{ 6, 37, 208 }, 1952 },
        { RGBColor{ 193, 164, 162 }, 2205 },
        { RGBColor{ 49, 8, 90 }, 2471 },
        { RGBColor{ 124, 149, 120 }, 2749 },
        { RGBColor{ 56, 69, 6 }, 3039 },
        { RGBColor{ 231, 177, 252 }, 3341 },
        { RGBColor{ 36, 76, 95 }, 3655 },
        { RGBColor{ 1, 99, 184 }, 3980 },
        { RGBColor{ 74, 223, 199 }, 4316 },
        { RGBColor{ 249, 125, 31 }, 4664 },
        { RGBColor{ 28, 93, 214 }, 5023 },
        { RGBColor{ 232, 117, 145 }, 5393 },
        { RGBColor{ 208, 158, 49 }, 5773 },
        { RGBColor{ 218, 109, 177 }, 6164 },
        { RGBColor{ 139, 83, 177 }, 6565 },
        { RGBColor{ 16, 36, 59 }, 6977 },
        { RGBColor{ 194, 157, 26 }, 7399 },
        { RGBColor{ 77, 236, 12 }, 7831 },
        { RGBColor{ 124, 244, 151 }, 8273 },
        { RGBColor{ 128, 195, 162 }, 8725 },
        { RGBColor{ 66, 36, 194 }, 9187 },
        { RGBColor{ 81, 151, 185 }, 9659 },
        { RGBColor{ 173, 75, 175 }, 10140 },
        { RGBColor{ 43, 182, 52 }, 10631 },
        { RGBColor{ 14, 242, 141 }, 11131 },
        { RGBColor{ 156, 203, 87 }, 11641 },
        { RGBColor{ 147, 89, 150 }, 12160 },
        { RGBColor{ 213, 199, 183 }, 12689 },
        { RGBColor{ 186, 255, 52 }, 13227 },
        { RGBColor{ 28, 158, 154 }, 13774 },
        { RGBColor{ 5, 5, 116 }, 14330 },
        { RGBColor{ 126, 123, 232 }, 14895 },
        { RGBColor{ 43, 162, 251 }, 15469 },
        { RGBColor{ 198, 143, 125 }, 16052 },
        { RGBColor{ 201, 157, 178 }, 16644 },
        { RGBColor{ 213, 151, 189 }, 17245 },
        { RGBColor{ 188, 117, 169 }, 17854 },
        { RGBColor{ 156, 189, 249 }, 18472 },
        { RGBColor{ 62, 23, 33 }, 19099 },
        { RGBColor{ 167, 205, 74 }, 19734 },
        { RGBColor{ 161, 181, 210 }, 20378 },
        { RGBColor{ 179, 167, 215 }, 21030 },
        { RGBColor{ 204, 102, 126 }, 21691 },
        { RGBColor{ 123, 49, 127 }, 22360 },
        { RGBColor{ 178, 48, 136 }, 23037 },
        { RGBColor{ 108, 112, 99 }, 23723 },
        { RGBColor{ 250, 152, 78 }, 24417 },
        { RGBColor{ 79, 167, 196 }, 25119 },
        { RGBColor{ 149, 167, 8 }, 25829 },
        { RGBColor{ 196, 29, 159 }, 26548 },
        { RGBColor{ 128, 26, 20 }, 27275 },
        { RGBColor{ 69, 49, 66 }, 28010 },
        { RGBColor{ 12, 42, 198 }, 28753 },
        { RGBColor{ 61, 62, 36 }, 29504 },
        { RGBColor{ 27, 94, 114 }, 30263 },
        { RGBColor{ 54, 218, 7 }, 31030 },
        { RGBColor{ 105, 89, 170 }, 31804 },
        { RGBColor{ 100, 110, 2 }, 32586 },
        { RGBColor{ 208, 198, 148 }, 33376 },
        { RGBColor{ 80, 208, 131 }, 34174 },
        { RGBColor{ 176, 89, 59 }, 34980 },
        { RGBColor{ 255, 64, 243 }, 35793 },
        { RGBColor{ 39, 226, 232 }, 36614 },
        { RGBColor{ 154, 100, 238 }, 37443 },
        { RGBColor{ 53, 103, 192 }, 38279 },
        { RGBColor{ 187, 41, 136 }, 39123 },
        { RGBColor{ 33, 84, 227 }, 39974 },
        { RGBColor{ 71, 167, 211 }, 40833 },
        { RGBColor{ 55, 191, 255 }, 41699 },
        { RGBColor{ 60, 165, 201 }, 42573 },
        { RGBColor{ 231, 206, 192 }, 43454 },
        { RGBColor{ 233, 224, 197 }, 44343 },
        { RGBColor{ 255, 129, 13 }, 45239 },
        { RGBColor{ 131, 222, 95 }, 46143 },
        { RGBColor{ 155, 249, 72 }, 47054 },
        { RGBColor{ 248, 129, 30 }, 47972 },
        { RGBColor{ 48, 239, 206 }, 48898 },
        { RGBColor{ 176, 224, 64 }, 49831 },
        { RGBColor{ 155, 12, 162 }, 50771 },
        { RGBColor{ 6, 144, 149 }, 51718 },
        { RGBColor{ 231, 208, 16 }, 52672 },
        { RGBColor{ 190, 66, 231 }, 53634 },
        { RGBColor{ 19, 17, 253 }, 54603 },
        { RGBColor{ 4, 34, 60 }, 55579 },
        { RGBColor{ 101, 23, 88 }, 56562 },
        { RGBColor{ 9, 191, 235 }, 57552 }
    };
    return Gradient{ colors };
}


RGBColor Gradient::get(float x) const
{
    if (colors.empty())
        return RGBColor();
    /*const auto [left, right, lerp] = getNeighbors(x);
    RGBColor lerped = lerpColors(left, right, lerp);
    return lerped;*/

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
