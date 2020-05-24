#ifndef FRACTALWIDGET_H
#define FRACTALWIDGET_H

#include "FractalZoomWidget.h"
#include <QPropertyAnimation>
#include <chrono>

class ViewportAnimation :
    public QPropertyAnimation
{
public:
    ViewportAnimation(QObject* parent = nullptr);

    QVariant interpolated(const QVariant& from, const QVariant& to,
                          qreal progress) const override;
};

class FractalWidget :
    public FractalZoomWidget
{
    bool rubberbanding = false;
    QRectF rubberband;

    bool dragging = false;
    float dragX, dragY;

    bool selectingPoint = false;
    float pointX, pointY;

    /// the target of an ongoing animation
    mnd::MandelViewport targetViewport;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastAnimUpdate;
public:
    FractalWidget(QWidget* parent = nullptr);

    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void wheelEvent(QWheelEvent* we) override;

    virtual void zoom(float factor) override;
    virtual void zoom(float factor, float fx, float fy);

    virtual void setViewport(const mnd::MandelViewport& viewport) override;
    virtual const mnd::MandelViewport& getViewport(void) const override;

    virtual void resizeGL(int w, int h) override;
    virtual void paintGL(void) override;

private:
    void newAnimation(void);
    void updateAnimations(void);
};

#endif // FRACTALWIDGET_H
