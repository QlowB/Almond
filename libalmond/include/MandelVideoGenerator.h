#ifndef MANDELVIDEOGENERATOR_H
#define MANDELVIDEOGENERATOR_H

#include "Mandel.h"
#include "Gradient.h"
#include "Bitmap.h"
#include <functional>

struct ExportVideoInfo
{
    /// the viewport at the start of the video
    mnd::MandelViewport start;
    /// the viewport at the end of the video
    mnd::MandelViewport end;
    /// Info struct to hold further data about the generation
    /// of mandelbrot images. The Viewport specified in this
    /// struct is ignored.
    mnd::MandelInfo mi;

    /// the gradient to use
    Gradient gradient;

    int fps;
    double zoomSpeed;

    std::string path;

    /// bitrate in kbps
    int bitrate;

    std::string preset;
};


struct MandelVideoProgressInfo
{
    int64_t framesExported;
    float progress;
};


class MandelVideoGenerator
{
public:
    using ProgressCallback = std::function<void(const MandelVideoProgressInfo&)>;
private:
    const ExportVideoInfo evi;
    std::vector<ProgressCallback> progressCallbacks;
    public:
    MandelVideoGenerator(const ExportVideoInfo& evi);

    void generate(mnd::MandelGenerator& gen);
    void addProgressCallback(ProgressCallback pc);

private:
    void callCallbacks(const MandelVideoProgressInfo& evi);
    Bitmap<RGBColor> overlay(const Bitmap<RGBColor>& outer,
                             const Bitmap<RGBColor>& inner,
                             long bw, long bh,
                             double scale, double oversizeFactor);
};

#endif // MANDELVIDEOGENERATOR_H
