#ifndef MANDELVIDEOGENERATOR_H
#define MANDELVIDEOGENERATOR_H

#include "Mandel.h"
#include "Gradient.h"
#include "Bitmap.h"

struct ExportVideoInfo
{
    mnd::MandelViewport start;
    mnd::MandelViewport end;
    Gradient gradient;

    int width;
    int height;
    int maxIterations;
    int fps;
    double zoomSpeed;

    std::string path;

    /// bitrate in kbps
    int bitrate;

    std::string preset;
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
