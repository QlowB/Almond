#include "FractalWidget.h"
#include <QMouseEvent>

#include <QOpenGLShaderProgram>
#include <QPainter>




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
        const auto& vp = getViewport();
        mnd::Real x = vp.x + vp.width * (float(me->pos().x()) / this->width());
        mnd::Real y = vp.y + vp.height * (float(me->pos().y()) / this->height());
        emit pointSelected(x, y);
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


void FractalWidget::setDisplayInfo(bool displayInfo)
{
    if (displayInfo != this->displayInfo) {
        this->displayInfo = displayInfo;
        update();
    }
}


void FractalWidget::selectJuliaPoint(void)
{
    this->selectingPoint = true;
    this->setMouseTracking(true);
    update();
}


void FractalWidget::stopSelectingPoint(void)
{
    this->selectingPoint = false;
    this->setMouseTracking(false);
    update();
}


void FractalWidget::resizeGL(int w, int h)
{
    FractalZoomWidget::resizeGL(w, h);
    targetViewport.height = targetViewport.width * h / w;
}


void FractalWidget::paintGL(void)
{
    updateAnimations();
    EscapeTimeVisualWidget::program->bind();
    FractalZoomWidget::paintGL();
    EscapeTimeVisualWidget::juliaPreviewer->bind();

    if (selectingPoint) {
        const auto& vp = getViewport();
        float jx = float(vp.x) + float(vp.width) * pointX / this->width();
        float jy = float(vp.y) + float(vp.height) * pointY / this->height();
        EscapeTimeVisualWidget::drawJulia(jx, jy);
        drawSelectingPoint();
    }
    if (rubberbanding)
        drawRubberband();
    if (displayInfo)
        drawDisplayInfo();
}


void FractalWidget::drawDisplayInfo(void)
{
    QPainter infoPainter{ this };

    const float DIST_FROM_BORDER = 15;
    float maxWidth = this->width() - 2 * DIST_FROM_BORDER;
    mnd::Real distPerPixel = getViewport().width / this->width();
    float log10 = (mnd::convert<float>(mnd::log(distPerPixel)) + ::logf(maxWidth)) / ::logf(10);
    mnd::Real displayDist = mnd::pow(mnd::Real(10), ::floor(log10));
    float pixels = mnd::convert<float>(displayDist / distPerPixel);
    int factor = 1;
    for (int i = 9; i > 1; i--) {
        if (pixels * i < maxWidth) {
            factor *= i;
            pixels *= i;
            displayDist *= i;
            break;
        }
    }

    std::stringstream dis;
    if (::abs(log10) < 3) {
        dis << mnd::convert<float>(displayDist);
    }
    else {
        dis << factor << "e" << int(::floor(log10));
    }

    if (maxWidth > 400) {
        dis << "; per pixel: " << distPerPixel;
    }

    float lineY = this->height() - DIST_FROM_BORDER;
    float lineXEnd = DIST_FROM_BORDER + pixels;

    infoPainter.setPen(Qt::white);
    infoPainter.setFont(QFont("Arial", 12));
    infoPainter.drawLine(QPointF{ DIST_FROM_BORDER, lineY }, QPointF{ lineXEnd, lineY });
    infoPainter.drawLine(QPointF{ DIST_FROM_BORDER, lineY }, QPointF{ DIST_FROM_BORDER, lineY - 5 });
    infoPainter.drawLine(QPointF{ lineXEnd, lineY }, QPointF{ lineXEnd, lineY - 5 });
    infoPainter.drawText(int(DIST_FROM_BORDER), int(lineY - 20), int(lineXEnd - DIST_FROM_BORDER), 20,
                         Qt::AlignCenter, QString::fromStdString(dis.str()));
}


void FractalWidget::drawSelectingPoint(void)
{
    QPainter pointPainter{ this };
    pointPainter.setPen(QColor{ 255, 255, 255 });
    pointPainter.drawLine(0, pointY, width(), pointY);
    pointPainter.drawLine(pointX, 0, pointX, height());
}


void FractalWidget::drawRubberband(void)
{
    QPainter rubberbandPainter{ this };
    rubberbandPainter.fillRect(rubberband, QColor{ 125, 140, 225, 120 });

    QPen pen{ QColor{ 100, 115, 200 } };
    pen.setWidth(2);
    rubberbandPainter.setPen(pen);

    rubberbandPainter.drawRect(rubberband);
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
