#include "ImageExport.h"
#include "Bitmap.h"
#include <png.h>
#include <cstdio>

namespace alm
{


void exportPng(const std::string& name, const ImageExportInfo& iei)
{
    if (iei.generator == nullptr) {
        throw "no generator";
    }
    mnd::MandelGenerator& generator = *iei.generator;
    FILE* file = fopen(name.c_str(), "wb");
    if(!file) exit(1);

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) exit(1);

    png_infop info = png_create_info_struct(png);
    if (!info) exit(1);

    if (setjmp(png_jmpbuf(png))) exit(1);

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

    long chunkHeight = 512;
    if (width <= 1024 && height <= 1024) {
        chunkHeight = 1024;
    }
    if (width >= 4096) {
        chunkHeight = 128;
    }

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
            return i >= iei.drawInfo.maxIter ? RGBColor{ 0, 0, 0 } : iei.gradient->get(i);
        });
        for (long i = 0; i < tmpHeight; i++) {
            rowPointers[i] = reinterpret_cast<png_byte*>(&coloredChunk.get(0, i));
        }
        png_write_rows(png, &rowPointers[0], tmpHeight);
    }

    png_write_end(png, NULL);

    fclose(file);

    png_destroy_write_struct(&png, &info);
}


}



