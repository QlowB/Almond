#ifndef MANDEL_MANDELUTIL_H
#define MANDEL_MANDELUTIL_H

#include "Real.h"

namespace mnd 
{
    struct MandelViewport;
    struct MandelInfo;
}


struct mnd::MandelViewport
{
    /// real part of the top left corner
    Real x = -2.1;

    /// imaginary part of the top left corner
    Real y = -1.5;

    /// real-part span of the picture to be generated
    Real width = 3;

    /// imaginary-part span of the picture to be generated
    Real height = 3;

    /*!
     * \brief adjusts the aspect ratio of the viewport, making sure
     *        the updated viewport contains all of the original one.
     */
    void adjustAspectRatio(double nwidth, double nheight);

    /*!
     * \brief make sure width and height are positive
     */
    void normalize(void);

    /*!
     * \brief zoom in around the center by a factor specified
     */
    void zoomCenter(float scale);
    void zoomCenter(const Real& scale);

    /*!
     * \brief zoom in around a specific point
     * \param x value between 0.0f and 1.0f
     * \param y value between 0.0f and 1.0f
     */
    void zoom(float scale, float x, float y);
    void zoom(const Real& scale, const Real& x, const Real& y);

    /*!
     * \brief returns a viewport where the whole mandelbrot set can be observed
     */
    static MandelViewport standardView(void);

    /*!
     * \brief returns a viewport with (0, 0) in the center
     */
    static MandelViewport centerView(void);

    inline Real right() const { return x + width; }
    inline Real bottom() const { return y + height; }
};


struct mnd::MandelInfo
{
    /// viewport
    MandelViewport view;

    /// width of the bitmap to be generated
    long bWidth;

    /// height of the bitmap to be generated
    long bHeight;

    /// maximum iterations
    long maxIter;

    /// smooth coloring
    bool smooth;

    bool julia = false;
    Real juliaX;
    Real juliaY;
};


#endif // MANDEL_MANDELUTIL_H
