#include "ImageExport.h"
#include "Bitmap.h"
#include <png.h>
#include <cstdio>

namespace alm
{

ImageExportException::ImageExportException(const std::string& err) :
    std::runtime_error{ err }
{
}

void exportPng(const ImageExportInfo& iei, std::function<void(float)> progressCallback)
{
    if (iei.generator == nullptr) {
        throw "no generator";
    }

    progressCallback(0.0f);

    mnd::MandelGenerator& generator = *iei.generator;
    FILE* file = fopen(iei.path.c_str(), "wb");
    if(!file)
        throw ImageExportException{ std::string("could not open file '") + iei.path + "'" };

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        throw ImageExportException{ "error creating png write struct" };

    png_infop info = png_create_info_struct(png);
    if (!info)
        throw ImageExportException{ "error creating png info struct" };

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        throw ImageExportException{ "error while creating png" };
    }

    png_init_io(png, file);

    const long width = iei.drawInfo.bWidth;
    const long height = iei.drawInfo.bHeight;
    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    long chunkHeight = height / 20;
    if (chunkHeight < 1)
        chunkHeight = 1;
    if (width >= 4096 && chunkHeight > 64)
        chunkHeight = 64;

    auto rowPointers = std::make_unique<png_byte*[]>(chunkHeight);
    for (long chunkY = 0; chunkY < height; chunkY += chunkHeight) {
        auto minimum = [] (const auto& a, const auto& b) { return a < b ? a : b; };
        long tmpHeight = minimum(chunkHeight, height - chunkY);
        mnd::MandelInfo chunkInfo = iei.drawInfo;
        chunkInfo.bHeight = tmpHeight;
        chunkInfo.view.y += chunkInfo.view.height * chunkY / height;
        chunkInfo.view.height *= mnd::Real(tmpHeight) / height;

        Bitmap<float> chunk(width, tmpHeight);
        generator.generate(chunkInfo, chunk.pixels.get());
        Bitmap<RGBColor> coloredChunk = chunk.map<RGBColor>([&iei] (float i) {
            return i >= iei.drawInfo.maxIter ? RGBColor{ 0, 0, 0 } : iei.gradient.get(i);
        });
        for (long i = 0; i < tmpHeight; i++) {
            rowPointers[i] = reinterpret_cast<png_byte*>(&coloredChunk.get(0, i));
        }
        png_write_rows(png, &rowPointers[0], tmpHeight);
        if (chunkY < height)
            progressCallback(100.0f * chunkY / height);
    }

    png_write_end(png, NULL);

    fclose(file);

    png_destroy_write_struct(&png, &info);
    progressCallback(100.0f);
}


}



