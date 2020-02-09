#ifndef MANDEL_MANDELUTIL_H
#define MANDEL_MANDELUTIL_H

namespace mnd 
{
    struct MandelViewport;
    struct MandelInfo;
}


struct mnd::MandelViewport
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

    /*!
     * \brief make sure width and height are positive
     */
    void normalize(void);

    /*!
     * \brief zoom in around the center by a factor specified
     */
    void zoomCenter(float scale);

    /*!
     * \brief returns a viewport where the whole mandelbrot set can be observed
     */
    static MandelViewport standardView(void);

    inline double right() const { return x + width; }
    inline double bottom() const { return y + height; }
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
    int maxIter;
};

#endif // MANDEL_MANDELUTIL_H
