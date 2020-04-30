#include "MandelVideoGenerator.h"
#include "VideoStream.h"
#include "Mandel.h"
#include <thread>
#include <cmath>

MandelVideoGenerator::MandelVideoGenerator(const ExportVideoInfo& evi) :
    evi{ evi }
{
}


void MandelVideoGenerator::addProgressCallback(ProgressCallback pc)
{
    progressCallbacks.push_back(std::move(pc));
}

void MandelVideoGenerator::generate(mnd::MandelGenerator& gen)
{
    VideoStream vs(evi.width, evi.height, evi.path, evi.bitrate, evi.fps, evi.preset.c_str());

    mnd::Real x = evi.end.x + evi.end.width / 2;
    mnd::Real y = evi.end.y + evi.end.height / 2;
    mnd::Real w = evi.start.width;
    mnd::Real h = evi.start.height;

    mnd::Real bigW = mnd::Real("1e+300");
    double bigFac = 1.0;
    Bitmap<RGBColor> big;
    Bitmap<RGBColor> small;

    int64_t frameCounter = 0;

    const float oversizeFactor = 2;
    const float sqrFactor = sqrt(oversizeFactor);
    //const mnd::Real invsqrt2 = mnd::Real(1.0) / mnd::sqrt(mnd::Real(2));

    mnd::MandelInfo mi;
    mi.bWidth = evi.width * oversizeFactor;
    mi.bHeight = evi.height * oversizeFactor;
    mi.maxIter = evi.maxIterations;

    bool first = true;

    while(w > evi.end.width || h > evi.end.height) {

        if (bigW > sqrt(oversizeFactor) * w) {
            mi.view = mnd::MandelViewport{ x - w/2, y - h/2, w, h };
            Bitmap<float> raw{ long(evi.width * oversizeFactor), long(evi.height * oversizeFactor) };
            Bitmap<float> rawSmall{ long(evi.width * oversizeFactor), long(evi.height * oversizeFactor) };
            mi.view.zoomCenter(oversizeFactor);
            gen.generate(mi, rawSmall.pixels.get());
            //mi.view.zoomCenter(sqrt(oversizeFactor));
            //gen.generate(mi, raw.pixels.get());
            //auto before = std::chrono::high_resolution_clock::now();
            if (first) {
                mi.view.zoomCenter(sqrt(oversizeFactor));
                gen.generate(mi, raw.pixels.get());
                small = raw.map<RGBColor>([&mi, this] (float i) {
                    return i >= mi.maxIter ? RGBColor{ 0, 0, 0 } : evi.gradient.get(i);
                });
            }
            big = std::move(small);
            small = rawSmall.map<RGBColor>([&mi, this] (float i) {
                return i >= mi.maxIter ? RGBColor{ 0, 0, 0 } : evi.gradient.get(i);
            });
            printf("recalced\n");
            /*mi.view.zoomCenter(0.5);
            gen.generate(mi, raw.pixels.get());
            small = raw.map<RGBColor>([] (float x) { return
                RGBColor{ uint8_t(::sin(x / 100) * 127 + 127), uint8_t(::sin(x / 213) * 127 + 127), uint8_t(::cos(x / 173) * 127 + 127) };
            });*/
            bigW = w;
            bigFac = 1.0;
            first = false;
        }

        vs.addFrame(overlay(big, small, evi.width, evi.height, bigFac, sqrt(oversizeFactor)));
        frameCounter++;
        MandelVideoProgressInfo mvpi{ frameCounter };
        callCallbacks(mvpi);

        w *= ::pow(0.99, evi.zoomSpeed);
        h *= ::pow(0.99, evi.zoomSpeed);
        bigFac *= ::pow(0.99, evi.zoomSpeed);
    }
}


void MandelVideoGenerator::callCallbacks(const MandelVideoProgressInfo& evi)
{
    for (auto& pc : progressCallbacks) {
        pc(evi);
    }
}

inline RGBColor lerpColors(const RGBColor& a, const RGBColor& b, double lerp)
{
    auto mklin = [] (double x) {
        return x;
    };
    auto unlin = [] (double x) {
        return x;
    };

    return RGBColor{
        uint8_t(a.r * lerp + b.r * (1 - lerp)),
        uint8_t(a.g * lerp + b.g * (1 - lerp)),
        uint8_t(a.b * lerp + b.b * (1 - lerp))
    };
}

inline RGBColor biliniear(const Bitmap<RGBColor>& img, double x, double y)
{
    int xfloor = int(::floor(x));
    int yfloor = int(::floor(y));
    int xceil = int(::ceil(x));
    int yceil = int(::ceil(y));

    double xLerp = x - xfloor;
    double yLerp = y - yfloor;

    RGBColor samples[2][2] = {
        {
            img.get(xfloor, yfloor),
            img.get(xfloor, yceil),
        },
        {
            img.get(xceil, yfloor),
            img.get(xceil, yceil),
        }
    };

    double r = 0, g = 0, b = 0;

    auto mklin = [] (double x) {
        return x;
    };
    auto unlin = [] (double x) {
        return x;
    };

    r += (1 - xLerp) * (1 - yLerp) * mklin(samples[0][0].r);
    r += (1 - xLerp) * yLerp * mklin(samples[0][1].r);
    r += xLerp * (1 - yLerp) * mklin(samples[1][0].r);
    r += xLerp * yLerp * mklin(samples[1][1].r);

    g += (1 - xLerp) * (1 - yLerp) * mklin(samples[0][0].g);
    g += (1 - xLerp) * yLerp * mklin(samples[0][1].g);
    g += xLerp * (1 - yLerp) * mklin(samples[1][0].g);
    g += xLerp * yLerp * mklin(samples[1][1].g);

    b += (1 - xLerp) * (1 - yLerp) * mklin(samples[0][0].b);
    b += (1 - xLerp) * yLerp * mklin(samples[0][1].b);
    b += xLerp * (1 - yLerp) * mklin(samples[1][0].b);
    b += xLerp * yLerp * mklin(samples[1][1].b);

    return RGBColor{ uint8_t(unlin(r)), uint8_t(unlin(g)), uint8_t(unlin(b)) };
}


inline RGBColor nearest(const Bitmap<RGBColor>& img, double x, double y)
{
    int xfloor = int(::floor(x));
    int yfloor = int(::floor(y));
    return img.get(xfloor, yfloor);
}


Bitmap<RGBColor> MandelVideoGenerator::overlay(const Bitmap<RGBColor>& outer,
                         const Bitmap<RGBColor>& inner, long bw, long bh,
                         double scale, double oversizeFactor)
{
    printf("%lf\n", scale);
    Bitmap<RGBColor> ret{ bw, bh };

    double outerLeft = outer.width * (1 - scale / oversizeFactor / oversizeFactor) / 2;
    double outerTop = outer.height * (1 - scale / oversizeFactor / oversizeFactor) / 2;
    double outerWidth = outer.width * scale / oversizeFactor / oversizeFactor;
    double outerHeight = outer.height * scale / oversizeFactor / oversizeFactor;

    double innerLeft = outer.width * (1 - scale / oversizeFactor) / 2;
    double innerTop = outer.height * (1 - scale / oversizeFactor) / 2;
    double innerWidth = outer.width * scale / oversizeFactor;
    double innerHeight = outer.height * scale / oversizeFactor;

            double lerpVal = ::log(1.0 / scale) / ::log(oversizeFactor);
            printf("lerpval: %f\n", lerpVal);
    auto before = std::chrono::high_resolution_clock::now();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < ret.height; i++) {
        for (int j = 0; j < ret.width; j++) {
            double newJ = outerLeft + outerWidth * j / ret.width;
            double newI = outerTop + outerHeight * i / ret.height;
            RGBColor a = biliniear(outer, newJ, newI);

            double innJ = innerLeft + innerWidth * j / ret.width;
            double innI = innerTop + innerHeight * i / ret.height;
            RGBColor b = biliniear(inner, innJ, innI);
            double lerpVal = -::log(scale) / ::log(oversizeFactor);
            RGBColor lerped = lerpColors(b, a, lerpVal);
            ret.get(j, i) = lerped;
        }
    }
    auto after = std::chrono::high_resolution_clock::now();
    //printf("gradient applied in: %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(after - before).count());
    fflush(stdout);
    /*for (int i = 0; i < ret.height * ret.width; i++) {
        ret.pixels[i] = outer.pixels[i];
    }*/

    return ret;
}
