#include "MandelVideoGenerator.h"
#include "VideoStream.h"
#include <thread>
#include <cmath>

MandelVideoGenerator::MandelVideoGenerator(const ExportVideoInfo& evi) :
    evi{ evi }
{
}


void MandelVideoGenerator::generate(void)
{
    mnd::MandelContext ctxt = mnd::initializeContext();
    mnd::Generator& gen = ctxt.getDefaultGenerator();//*ctxt.getDevices()[0].getGeneratorDouble();
    mnd::MandelInfo mi;
    mi.bWidth = evi.width * 2;
    mi.bHeight = evi.height * 2;
    mi.maxIter = evi.maxIterations;

    VideoStream vs(evi.width, evi.height, evi.path);

    double x = evi.end.x + evi.end.width / 2;
    double y = evi.end.y + evi.end.height / 2;
    double w = evi.start.width;
    double h = evi.start.height;

    double bigW = 10000000000000000.0;
    double bigFac = 1.0;
    Bitmap<RGBColor> big;
    Bitmap<RGBColor> small;

    while(w > evi.end.width || h > evi.end.height) {
        mi.view = mnd::MandelViewport{ x - w/2, y - h/2, w, h };

        if (bigW > 2 * w) {
            Bitmap<float> raw{ evi.width * 2, evi.height * 2 };
            gen.generate(mi, raw.pixels.get());
            auto before = std::chrono::high_resolution_clock::now();
            big = raw.map<RGBColor>([&mi, this] (float i) {
                return i >= mi.maxIter ? RGBColor{ 0,0,0 } : evi.gradient.get(i);
            });
            /*mi.view.zoomCenter(0.5);
            gen.generate(mi, raw.pixels.get());
            small = raw.map<RGBColor>([] (float x) { return
                RGBColor{ uint8_t(::sin(x / 100) * 127 + 127), uint8_t(::sin(x / 213) * 127 + 127), uint8_t(::cos(x / 173) * 127 + 127) };
            });*/
            bigW = w;
            bigFac = 1.0;
        }

        vs.addFrame(overlay(big, small, bigFac));

        w *= 0.99;
        h *= 0.99;
        bigFac *= 0.99;
    }
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
                         const Bitmap<RGBColor>& inner, double scale)
{
    printf("%lf\n", scale);
    Bitmap<RGBColor> ret{ outer.width / 2, outer.height / 2 };
    double newW = outer.width * scale * 2;
    double newH = outer.height * scale * 2;
    double newX = outer.width * (1 - scale) / 2;
    double newY = outer.height * (1 - scale) / 2;

    auto before = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (int i = 0; i < ret.height; i++) {
        for (int j = 0; j < ret.width; j++) {
            double newJ = newX + j * newW / outer.width;
            double newI = newY + i * newH / outer.height;
            RGBColor a = biliniear(outer, newJ, newI);
            ret.get(j, i) = a;
        }
    }
    auto after = std::chrono::high_resolution_clock::now();
    printf("gradient applied in: %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(after - before).count());
    fflush(stdout);
    /*for (int i = 0; i < ret.height * ret.width; i++) {
        ret.pixels[i] = outer.pixels[i];
    }*/

    return ret;
}
