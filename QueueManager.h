#pragma once
#ifndef QUEUEMANAGER_H_
#define QUEUEMANAGER_H_

#include <cinttypes>
#include <vector>
#include <future>
#include "Bitmap.h"

struct MandelViewport
{
    /// real part of the top left corner
    double x = -2.1;

    /// imaginary part of the top left corner
    double y = -1.5;

    /// real-part span of the picture to be generated
    double width = 3;

    /// imaginary-part span of the picture to be generated
    double height = 3;

    /*!
     * \brief adjusts the aspect ratio of the viewport, making sure
     *        the updated viewport contains all of the original one.
     */
    void adjustAspectRatio(double nwidth, double nheight);
};

struct MandelInfo
{
    /// viewport
    MandelViewport view;

    /// width of the bitmap to be generated
    long bWidth;

    /// height of the bitmap to be generated
    long bHeight;
    
    /// maximum iterations
    int maxIter;
};


class MandelGenerator
{
public:
    MandelGenerator(void) = default;
    virtual ~MandelGenerator(void);

    virtual Bitmap<RGBColor> generate(const MandelInfo& mandelInfo);
    virtual Bitmap<float> generateRaw(const MandelInfo& info) = 0;
};


class QueueManager
{
public:
    QueueManager(void);
    ~QueueManager(void);

    std::future<Bitmap<RGBColor>> generate(const MandelInfo& mandelInfo);
};

#endif // QUEUEMANAGER_H_

