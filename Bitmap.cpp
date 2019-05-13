#include "Bitmap.h"
#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#else
// for Linux platform, plz make sure the size of data type is correct for BMP spec.
// if you use this on Windows or other platforms, plz pay attention to this.
typedef int LONG;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

// __attribute__((packed)) on non-Intel arch may cause some unexpected error, plz be informed.

typedef struct tagBITMAPFILEHEADER
{
    WORD    bfType; // 2  /* Magic identifier */
    DWORD   bfSize; // 4  /* File size in bytes */
    WORD    bfReserved1; // 2
    WORD    bfReserved2; // 2
    DWORD   bfOffBits; // 4 /* Offset to image data, bytes */ 
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD    biSize; // 4 /* Header size in bytes */
    LONG     biWidth; // 4 /* Width of image */
    LONG     biHeight; // 4 /* Height of image */
    WORD     biPlanes; // 2 /* Number of colour planes */
    WORD     biBitCount; // 2 /* Bits per pixel */
    DWORD    biCompression; // 4 /* Compression type */
    DWORD    biSizeImage; // 4 /* Image size in bytes */
    LONG     biXPelsPerMeter; // 4
    LONG     biYPelsPerMeter; // 4 /* Pixels per meter */
    DWORD    biClrUsed; // 4 /* Number of colours */ 
    DWORD    biClrImportant; // 4 /* Important colours */ 
} __attribute__((packed)) BITMAPINFOHEADER;
#endif
#include <cstdlib>

template struct Bitmap<RGBColor>;



template<>
template<>
void Bitmap<RGBColor>::createPng<RGBColor>(const std::string& path) const
{
    std::FILE* out = std::fopen(path.c_str(), "wb");

    BITMAPFILEHEADER bmfh;
    bmfh.bfType = 0x4d42;
    bmfh.bfSize = 14 + 40 + (width * height * 3);
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = 14 + 40;

    BITMAPINFOHEADER bmih;
    memset(&bmih, 0, 40);
    bmih.biSize = 40;
    bmih.biWidth = width;
    bmih.biHeight = -long(height);
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = 0;

    std::fwrite(&bmfh, sizeof bmfh, 1, out);
    std::fwrite(&bmih, sizeof bmih, 1, out);

    size_t lineLength = (width * 3 + 3) & ~3;

    std::unique_ptr<unsigned char[]> linebuf = std::make_unique<unsigned char[]>(lineLength);
    memset(linebuf.get(), 0, lineLength);
    for (long i = 0; i < height; i++) {
        for (long j = 0; j < width; j++) {
            linebuf[j * 3 + 0] = pixels[i * height + j].b;
            linebuf[j * 3 + 1] = pixels[i * height + j].g;
            linebuf[j * 3 + 2] = pixels[i * height + j].r;
        }
        std::fwrite(linebuf.get(), 1, lineLength, out);
    }
    std::fclose(out);
}
