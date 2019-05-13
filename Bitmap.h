#pragma once
#ifndef BITMAP_H_
#define BITMAP_H_

#include "Color.h"
#include <memory>
#include <string>
#include <functional>

template<typename Pixel>
struct Bitmap
{
    long width, height;
    std::unique_ptr<Pixel[]> pixels;
public:
    Bitmap(void) :
        width{ 0 },
        height{ 0 },
        pixels{ 0 }
    {
    }


    Bitmap(long width, long height) :
        width{ width }, height{ height },
        pixels{ std::make_unique<Pixel[]>(width * height) }
    {
    }

    Bitmap(Bitmap&&) = default;
    Bitmap& operator = (Bitmap&&) = default;

    ~Bitmap() = default;

    template<typename P = Pixel>
    auto createPng(const std::string& path) const -> typename std::enable_if<std::is_same<P, RGBColor>::value>::type;

    template<typename T>
    Bitmap<T> map(std::function<T(Pixel)> f) const {
        Bitmap<T> b{ width, height };
        for (::size_t i = 0; i < width * height; i++) {
            b.pixels[i] = f(pixels[i]);
        }
        return b;
    }

    Pixel& get(long x, long y)
    {
        return pixels[x + y * width];
    }

    const Pixel& get(long x, long y) const
    {
        return pixels[x + y * width];
    }

    void print(void)
    {
        for (size_t i = 0; i < width * height; i++) {
            printf("%03d ", int(pixels[i].r));
        }
    }
};

#endif // BITMAP_H_
