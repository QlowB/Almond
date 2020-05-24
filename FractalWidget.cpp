#include "FractalWidget.h"
#include <QMouseEvent>



Q_DECLARE_METATYPE(mnd::MandelViewport)


ViewportAnimation::ViewportAnimation(QObject* parent) :
    QPropertyAnimation{ parent }
{
}


QVariant ViewportAnimation::interpolated(const QVariant& from, const QVariant& to,
                      qreal progress) const
{
    const mnd::MandelViewport& a = from.value<mnd::MandelViewport>();
    const mnd::MandelViewport& b = to.value<mnd::MandelViewport>();

    auto retVal = mnd::MandelViewport {
        a.x * (1 - progress) + b.x * progress,
        a.y * (1 - progress) + b.y * progress,
        a.width * (1 - progress) + b.width * progress,
        a.height * (1 - progress) + b.height * progress,
    };

    return QVariant::fromValue(retVal);
}


FractalWidget::FractalWidget(QWidget* parent) :
    FractalZoomWidget{ parent }
{
}


void FractalWidget::mousePressEvent(QMouseEvent* me)
{
    QOpenGLWidget::mousePressEvent(me);
    if (me->button() == Qt::RightButton) {
        rubberbanding = true;
        rubberband.setCoords(me->x(), me->y(), me->x(), me->y());
        update();
        me->accept();
    }
    else if (me->button() == Qt::LeftButton) {
        dragging = true;
        dragX = me->x();
        dragY = me->y();
        me->accept();
    }
}


void FractalWidget::mouseMoveEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseMoveEvent(me);
    if (rubberbanding) {
        QRectF& rect = rubberband;
        double aspect = double(geometry().width()) / geometry().height();
        rect.setBottomRight(QPoint(me->x(), me->y()));
        if (rect.width() > rect.height() * aspect)
            rect.setHeight(rect.width() / aspect);
        else
            rect.setWidth(rect.height() * aspect);

        update();
    }
    else if (selectingPoint) {
        pointX = me->x();
        pointY = me->y();
        update();
    }
    else if (dragging) {
        double deltaX = me->x() - dragX;
        double deltaY = me->y() - dragY;

        auto& viewport = mandelInfo.view;
        viewport.x -= deltaX * viewport.width / this->width();
        viewport.y -= deltaY * viewport.height / this->height();
        targetViewport = viewport;
        dragX = me->x(); dragY = me->y();

        update();
    }
    me->accept();
}


void FractalWidget::mouseReleaseEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseReleaseEvent(me);
    if (rubberbanding) {
        QRect rect = rubberband.toRect();
        if(rect.width() != 0 && rect.height() != 0) {
            QRect full = this->geometry();

            auto& viewport = targetViewport;
            viewport.x += mnd::Real(rect.left()) * viewport.width / full.width();
            viewport.y += mnd::Real(rect.top()) * viewport.height / full.height();
            viewport.width *= mnd::Real(rect.width()) / full.width();
            viewport.height *= mnd::Real(rect.height()) / full.height();
            viewport.normalize();
            viewport.adjustAspectRatio(getResolutionX(), getResolutionY());
            newAnimation();
            //currentViewport = viewport;
        }
        update();
        rubberbanding = false;
    }
    else if (selectingPoint) {
        selectingPoint = false;
        this->setMouseTracking(false);
        /*mnd::Real x = currentViewport.x + currentViewport.width * mnd::convert<mnd::Real>(float(me->x()) / width());
        mnd::Real y = currentViewport.y + currentViewport.height * mnd::convert<mnd::Real>(float(me->y()) / height());
        emit pointSelected(x, y);*/
        update();
    }
    dragging = false;
}

void FractalWidget::wheelEvent(QWheelEvent* we)
{
    QOpenGLWidget::wheelEvent(we);
    float x = float(we->x()) / this->width();
    float y = float(we->y()) / this->height();
    float scale = ::powf(0.9975f, we->angleDelta().y());
    //mandelInfo.view.zoom(scale, x, y);
    zoom(scale, x, y);
    //if (!we->pixelDelta().isNull())
    //    this->currentViewport = this->viewport;
    we->accept();
}


void FractalWidget::zoom(float factor)
{
    targetViewport.zoomCenter(factor);
    newAnimation();
    update();
}


void FractalWidget::zoom(float factor, float fx, float fy)
{
    targetViewport.zoom(factor, fx, fy);
    newAnimation();
    update();
    /*viewportSmoother = new ViewportAnimation(this);
    viewportSmoother->setStartValue(QVariant::fromValue(getViewport()));
    viewportSmoother->setEndValue(QVariant::fromValue(newVp));
    viewportSmoother->setTargetObject(this);
    viewportSmoother->setPropertyName("viewport");
    viewportSmoother->setDuration(200);
    viewportSmoother->setEasingCurve(QEasingCurve::OutExpo);
    viewportSmoother->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);*/
}


void FractalWidget::setViewport(const mnd::MandelViewport& viewport)
{
    FractalZoomWidget::setViewport(viewport);
    targetViewport = mandelInfo.view;
    update();
}


const mnd::MandelViewport& FractalWidget::getViewport(void) const
{
    return mandelInfo.view;
}


void FractalWidget::resizeGL(int w, int h)
{
    FractalZoomWidget::resizeGL(w, h);
    targetViewport.adjustAspectRatio(w, h);
}


void FractalWidget::paintGL(void)
{
    FractalZoomWidget::paintGL();
    updateAnimations();
}


void FractalWidget::newAnimation(void)
{
    auto now = std::chrono::high_resolution_clock::now();
    lastAnimUpdate = now;
}


void FractalWidget::updateAnimations(void)
{
    auto& currentViewport = mandelInfo.view;
    if (mnd::abs(currentViewport.width / targetViewport.width - 1.0) < 1e-3
            && mnd::abs(currentViewport.height / targetViewport.height - 1.0) < 1e-3) {
        // animation finished
        currentViewport = targetViewport;
    }
    else {
        auto now = std::chrono::high_resolution_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimUpdate).count();
        const mnd::Real factor = mnd::Real(::pow(0.97, millis));
        const mnd::Real one(1.0);

        currentViewport.x = currentViewport.x * factor + targetViewport.x * (one - factor);
        currentViewport.y = currentViewport.y * factor + targetViewport.y * (one - factor);
        currentViewport.width = currentViewport.width * factor + targetViewport.width * (one - factor);
        currentViewport.height = currentViewport.height * factor + targetViewport.height * (one - factor);

        lastAnimUpdate = now;
        emit update();
    }
}
