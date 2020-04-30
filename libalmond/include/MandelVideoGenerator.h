#ifndef MANDELVIDEOGENERATOR_H
#define MANDELVIDEOGENERATOR_H

#include "MandelUtil.h"
#include "Gradient.h"
#include "Bitmap.h"
#include <functional>

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


struct MandelVideoProgressInfo
{
    int framesExported;
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

    void generate(void);
    void addProgressCallback(ProgressCallback pc);

private:
    void callCallbacks(const MandelVideoProgressInfo& evi);
    Bitmap<RGBColor> overlay(const Bitmap<RGBColor>& outer,
                             const Bitmap<RGBColor>& inner,
                             long bw, long bh,
                             double scale, double oversizeFactor);
};

#endif // MANDELVIDEOGENERATOR_H
