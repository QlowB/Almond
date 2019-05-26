#ifndef MANDELVIDEOGENERATOR_H
#define MANDELVIDEOGENERATOR_H

#include "Mandel.h"
#include "Bitmap.h"

struct ExportVideoInfo
{
    mnd::MandelViewport start;
    mnd::MandelViewport end;

    int width;
    int height;
    int maxIterations;

    std::string path;
};


class MandelVideoGenerator
{
    const ExportVideoInfo evi;
public:
    MandelVideoGenerator(const ExportVideoInfo& evi);

    void generate(void);

private:
    Bitmap<RGBColor> overlay(const Bitmap<RGBColor>& outer,
                             const Bitmap<RGBColor>& inner,
                             double scale);
};

#endif // MANDELVIDEOGENERATOR_H
