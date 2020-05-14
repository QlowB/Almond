#include "ImageExport.h"
#include "Bitmap.h"
#include <boost/endian/buffers.hpp>
#include <cstdio>

#ifdef WITH_LIBPNG
extern "C" {
#   include <png.h>
}
#endif // WITH_LIBPNG

#ifdef WITH_LIBJPEG
extern "C" {
#   include <jpeglib.h>
}
#endif // WITH_LIBJPEG


namespace alm
{

#ifdef WITH_LIBPNG
void exportPng(const ImageExportInfo& iei,
    std::function<void(float)> progressCallback,
    std::function<bool(void)> cancelCallback);
#endif // WITH_LIBPNG

#ifdef WITH_LIBJPEG
void exportJpeg(const ImageExportInfo& iei,
    std::function<void(float)> progressCallback,
    std::function<bool(void)> cancelCallback);
#endif // WITH_LIBJPEG

void exportBmp(const ImageExportInfo& iei,
    std::function<void(float)> progressCallback,
    std::function<bool(void)> cancelCallback);


const std::vector<ImageFormat> supportedImageFormats = {
    ImageFormat::BMP,
#ifdef WITH_LIBPNG
    ImageFormat::PNG,
#endif // WITH_LIBPNG
#ifdef WITH_LIBJPEG
    ImageFormat::JPEG,
#endif // WITH_LIBJPEG
};

bool supportsImageFormat(ImageFormat imgf)
{
    for (auto f : supportedImageFormats) {
        if (imgf == f)
            return true;
    }
    return false;
}

ImageExportException::ImageExportException(const std::string& err) :
    std::runtime_error{ err }
{
}

void exportImage(const ImageExportInfo& iei,
                 std::function<void(float)> progressCallback,
                 std::function<bool(void)> cancelCallback)
{
    auto hasSuffix = [] (const std::string& path, const std::string& suffix) -> bool
    {
        return path.size() >= suffix.size() &&
               path.compare(path.size() - suffix.size(),
                        suffix.size(), suffix) == 0;
    };
    if (hasSuffix(iei.path, ".bmp") ||
        hasSuffix(iei.path, ".dib")) {
        exportBmp(iei, progressCallback, cancelCallback);
    }
#ifdef WITH_LIBPNG
    else if (hasSuffix(iei.path, ".png")) {
        exportPng(iei, progressCallback, cancelCallback);
    }
#endif // WITH_LIBPNG
#ifdef WITH_LIBJPEG
    else if (hasSuffix(iei.path, ".jpg") ||
             hasSuffix(iei.path, ".jpeg")) {
        exportJpeg(iei, progressCallback, cancelCallback);
    }
#endif // WITH_LIBJPEG
    else {
        throw ImageExportException{ "Could not guess image format" };
    }
}


#ifdef WITH_LIBPNG
void exportPng(const ImageExportInfo& iei,
               std::function<void(float)> progressCallback,
               std::function<bool(void)> cancelCallback)
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
    while (width * chunkHeight > 512 * 512)
        chunkHeight /= 2;

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

        if (cancelCallback()) {
            fclose(file);
            png_destroy_write_struct(&png, &info);
            return;
        }
    }

    png_write_end(png, NULL);

    fclose(file);

    png_destroy_write_struct(&png, &info);
    progressCallback(100.0f);
}
#endif // WITH_LIBPNG


#ifdef WITH_LIBJPEG
void exportJpeg(const ImageExportInfo& iei,
                std::function<void(float)> progressCallback,
                std::function<bool(void)> cancelCallback)
{
    if (iei.generator == nullptr) {
        throw "no generator";
    }

    progressCallback(0.0f);

    mnd::MandelGenerator& generator = *iei.generator;
    FILE* file = fopen(iei.path.c_str(), "wb");
    if(!file)
        throw ImageExportException{ std::string("could not open file '") + iei.path + "'" };

    const long width = iei.drawInfo.bWidth;
    const long height = iei.drawInfo.bHeight;

    struct jpeg_compress_struct jpegInfo;
    struct jpeg_error_mgr jerr;

    int rowLength = width * 3;

    jpegInfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = [](j_common_ptr cinfo) {
        auto errmsg = std::make_unique<char[]>(JMSG_LENGTH_MAX);
        (cinfo->err->format_message)(cinfo, errmsg.get());
        throw ImageExportException{ std::string("libjpeg error: ") + errmsg.get() };
    };
    jpeg_create_compress(&jpegInfo);

    jpeg_stdio_dest(&jpegInfo, file);

    jpegInfo.image_width = width;
    jpegInfo.image_height = height;
    jpegInfo.input_components = 3;
    jpegInfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jpegInfo);
    jpeg_set_quality(&jpegInfo, iei.options.jpegQuality, TRUE);
    jpeg_start_compress(&jpegInfo, TRUE);


    long chunkHeight = height / 20;
    if (chunkHeight < 1)
        chunkHeight = 1;
    while (width * chunkHeight > 512 * 512)
        chunkHeight /= 2;

    auto rowPointers = std::make_unique<JSAMPROW[]>(chunkHeight);
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
            rowPointers[i] = reinterpret_cast<JSAMPROW>(&coloredChunk.get(0, i));
        }
        int written = jpeg_write_scanlines(&jpegInfo, rowPointers.get(), tmpHeight);
        if (written < tmpHeight) {
            throw ImageExportException{ "error writing jpeg rows" };
        }
        if (chunkY < height)
            progressCallback(100.0f * chunkY / height);

        if (cancelCallback()) {
            fclose(file);
            jpeg_destroy_compress(&jpegInfo);
            return;
        }
    }

    jpeg_finish_compress(&jpegInfo);
    fclose(file);
    jpeg_destroy_compress(&jpegInfo);
    progressCallback(100.0f);
}

#endif // WITH_LIBJPEG


void exportBmp(const ImageExportInfo& iei,
               std::function<void(float)> progressCallback,
               std::function<bool(void)> cancelCallback)
{
    if (iei.generator == nullptr) {
        throw "no generator";
    }

    progressCallback(0.0f);

    mnd::MandelGenerator& generator = *iei.generator;
    FILE* file = fopen(iei.path.c_str(), "wb");
    if(!file)
        throw ImageExportException{ std::string("could not open file '") + iei.path + "'" };

    const long width = iei.drawInfo.bWidth;
    const long height = iei.drawInfo.bHeight;

    // bmp needs rows to be 4-byte-aligned
    const long rowLength = (width * 3 + 3) & ~0x03;

    using namespace boost::endian;

    const int fileHeaderSize = 14;
    const int infoHeaderSize = 40;
    const int pixelSize = rowLength * height;

    // file header
    little_int8_buf_t fhType[2];
    little_uint32_buf_t fhSize;
    little_uint16_buf_t fhReserved1;
    little_uint16_buf_t fhReserved2;
    little_uint32_buf_t fhOffset;
    fhType[0] = 0x42; // 'B'
    fhType[1] = 0x4D; // 'M'
    fhSize = fileHeaderSize + infoHeaderSize + pixelSize;
    fhReserved1 = 0;
    fhReserved1 = 2;
    fhOffset = fileHeaderSize + infoHeaderSize;

    // info header
    little_uint32_buf_t ihSize;
    little_int32_buf_t ihWidth;
    little_int32_buf_t ihHeight; // for top-down, height must be negative
    little_uint16_buf_t ihPlanes;
    little_uint16_buf_t ihBitCount;
    little_uint32_buf_t ihCompression; // 0 = no compression
    little_uint32_buf_t ihSizeImage; // for uncompressed bmps 0 can be set here
    little_int32_buf_t ihXPixelsPerMeter;
    little_int32_buf_t ihYPixelsPerMeter;
    little_uint32_buf_t ihClrUsed;
    little_uint32_buf_t ihClrImportant;
    ihSize = infoHeaderSize;
    ihWidth = width;
    ihHeight = -height; // for top-down, height must be negative
    ihPlanes = 1;
    ihBitCount = 24;
    ihCompression = 0; // 0 = no compression
    ihSizeImage = 0; // for uncompressed bmps 0 can be set here
    ihXPixelsPerMeter = 0;
    ihYPixelsPerMeter = 0;
    ihClrUsed = 0;
    ihClrImportant = 0;
    
    auto writeCmp = [&file] (auto& buf) {
        std::fwrite(&buf, sizeof(buf), 1, file);
    };

    writeCmp(fhType[0]);
    writeCmp(fhType[1]);
    writeCmp(fhSize);
    writeCmp(fhReserved1);
    writeCmp(fhReserved2);
    writeCmp(fhOffset);

    writeCmp(ihSize);
    writeCmp(ihWidth);
    writeCmp(ihHeight);
    writeCmp(ihPlanes);
    writeCmp(ihBitCount);
    writeCmp(ihCompression);
    writeCmp(ihSizeImage);
    writeCmp(ihXPixelsPerMeter);
    writeCmp(ihYPixelsPerMeter);
    writeCmp(ihClrUsed);
    writeCmp(ihClrImportant);

    long chunkHeight = height / 20;
    if (chunkHeight < 1)
        chunkHeight = 1;
    while (width * chunkHeight > 512 * 512)
        chunkHeight /= 2;

    for (long chunkY = 0; chunkY < height; chunkY += chunkHeight) {
        auto minimum = [] (const auto& a, const auto& b) { return a < b ? a : b; };
        long tmpHeight = minimum(chunkHeight, height - chunkY);
        mnd::MandelInfo chunkInfo = iei.drawInfo;
        chunkInfo.bHeight = tmpHeight;
        chunkInfo.view.y += chunkInfo.view.height * chunkY / height;
        chunkInfo.view.height *= mnd::Real(tmpHeight) / height;

        Bitmap<float> chunk(width, tmpHeight);
        generator.generate(chunkInfo, chunk.pixels.get());
        for (long i = 0; i < tmpHeight; i++) {
            for (long j = 0; j < width; j++) {
                float iters = chunk.get(j, i);
                RGBColor color =
                    iters >= iei.drawInfo.maxIter ?
                        RGBColor{ 0, 0, 0 } :
                        iei.gradient.get(iters);
                uint8_t bgr[3] = { color.b, color.g, color.r };
                fwrite(bgr, 1, 3, file);
            }

            // write line padding
            const uint8_t zeroes[] = { 0, 0, 0, 0 };
            fwrite(zeroes, 1, rowLength - width * 3, file);
        }
        if (chunkY < height)
            progressCallback(100.0f * chunkY / height);

        if (cancelCallback()) {
            fclose(file);
            return;
        }
    }

    fflush(file);
    fclose(file);
    progressCallback(100.0f);
}
}



