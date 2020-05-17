#include "GradientWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QColorDialog>
#include <QStyle>

#include <algorithm>
#include <cmath>

GradientWidget::GradientWidget(QWidget* parent) :
    QWidget{ parent }
{
    dragging = false;
    selectedHandle = -1;
    mouseOver = -1;

    setMouseTracking(true);
}


const QVector<std::pair<float, QColor>>&
GradientWidget::getGradient(void) const
{
    return points;
}


void GradientWidget::setGradient(QVector<std::pair<float, QColor>> vec)
{
    points = std::move(vec);
}


QColor lerp(const QColor& a, const QColor& b, float v)
{
    float ar = a.redF();
    float ag = a.greenF();
    float ab = a.blueF();

    float br = b.redF();
    float bg = b.greenF();
    float bb = b.blueF();

    const float gamma = 2.2;

    ar = std::pow(ar, gamma);
    ag = std::pow(ag, gamma);
    ab = std::pow(ab, gamma);

    br = std::pow(br, gamma);
    bg = std::pow(bg, gamma);
    bb = std::pow(bb, gamma);

    float nr = br * v + (1 - v) * ar;
    float ng = bg * v + (1 - v) * ag;
    float nb = bb * v + (1 - v) * ab;

    nr = std::pow(nr, 1/gamma);
    ng = std::pow(ng, 1/gamma);
    nb = std::pow(nb, 1/gamma);

    return QColor{ int(255 * nr), int(255 * ng), int(255 * nb) };
}

void GradientWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter{ this };

    std::vector<int> orderedIndices(points.size());
    for (int i = 0; i < points.size(); i++)
        orderedIndices.push_back(i);

    std::sort(orderedIndices.begin(), orderedIndices.end(),
        [this] (int l, int r) {
            return points[l].first < points[r].first;
        });

    QRect gradientRect = getGradientRect();
    QStyleOption frameOptions;
    frameOptions.init(this);
    frameOptions.rect = gradientRect;
    frameOptions.state |= QStyle::State_Sunken;
    style()->drawPrimitive(
                QStyle::PrimitiveElement::PE_Frame, &frameOptions, &painter, this);

    int fhmargins = style()->pixelMetric(QStyle::PixelMetric::PM_FocusFrameHMargin);
    int fvmargins = style()->pixelMetric(QStyle::PixelMetric::PM_FocusFrameVMargin);
    if (fhmargins == -1) {
        fhmargins = fvmargins = 3;
    }

    QLinearGradient gradient;
    gradient.setStart(0, gradientRect.top());
    gradient.setFinalStop(0, gradientRect.bottom());

    // adjust rect to have small margins, so the frame
    // around the gradient is visible
    gradientRect.adjust(fhmargins, fvmargins, -fhmargins, -fvmargins);
    float lastPoint = 0;
    QColor lastColor = QColor{ 0, 0, 0 };
    // traverse gradient in order and interpolate in linear
    // RGB space to avoid interpolating in sRGB
    for (int i = 0; i < orderedIndices.size(); i++) {
        int index = orderedIndices[i];
        auto& [point, color] = points[index];
        int m = 5;
        if (i > 0) {
            for (int i = 0; i < m; i++) {
                float v = float(i) / m;
                gradient.setColorAt(lastPoint + (point - lastPoint) / m * i,
                                    lerp(lastColor, color, v));
            }
        }
        gradient.setColorAt(point, color);
        lastPoint = point;
        lastColor = color;
    }
    QBrush brush{ gradient };
    painter.fillRect(gradientRect, brush);

    QStyleOption so;
    so.init(this);
    int index = 0;
    for (auto& [point, color] : points) {
        QRect r = getHandleRect(index);
        so.rect = r;
        if (dragging && selectedHandle == index)
            so.state |= QStyle::State_Sunken;
        else
            so.state &= ~QStyle::State_Sunken;
        if (mouseOver == index)
            so.state |= QStyle::State_MouseOver;
        else
            so.state &= ~QStyle::State_MouseOver;
        so.palette.setColor(QPalette::ColorRole::Button, color);
        so.palette.setColor(QPalette::ColorRole::Background, color);
        style()->drawPrimitive(QStyle::PrimitiveElement::PE_PanelButtonTool, &so, &painter, this);
        index++;
    }
    /*for (auto&[point, color] : points) {
        QStyleOptionSlider qsos;
        qsos.rect = QRect{ 100, static_cast<int>(point * height() - 10), 20, 20 };
        qsos.orientation = Qt::Vertical;
        qsos.subControls = QStyle::SC_SliderHandle;
        if (selectedHandle == index) {
            qsos.state |= QStyle::State_Sunken;
        }
        else {
            qsos.state &= ~QStyle::State_Sunken;
        }
        style()->drawComplexControl(QStyle::CC_Slider, &qsos, &painter, this);
        index++;
    }*/
    /*

    QPen pen(Qt::red);
    pen.setWidth(10);
    painter.setPen(pen);
    painter.drawRect(30, 30, 50, 50);

    painter.fillRect(QRect(0, 0, width(), height()), QColor{ 100, 20, 20 });
    qDebug(std::to_string(width()).c_str());
    */
}


void GradientWidget::mousePressEvent(QMouseEvent* e)
{
    int handle = handleAtPos(e->pos());

    if (handle != -1) {
        selectedHandle = handle;
        dragging = true;
        selectOffsetY = e->y() - gradValToHandleY(
                    points[handle].first);
        update();
        e->accept();
    }
    else {
        e->ignore();
    }
}


void GradientWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (dragging) {
        dragging = false;
        selectedHandle = -1;
        update();
        e->accept();
    }
    else {
        e->ignore();
    }
}


void GradientWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (dragging) {
        float newVal = handleYToGradVal(e->y() - selectOffsetY);
        newVal = std::clamp(newVal, 0.0f, 1.0f);
        points[selectedHandle].first = newVal;
        update();
        e->accept();
    }
    else {
        int handle = handleAtPos(e->pos());
        bool needsUpdate = false;
        if (mouseOver != handle)
            needsUpdate = true;
        mouseOver = handle;
        e->accept();
        if (needsUpdate)
            update();
    }
}


void GradientWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    int handle = handleAtPos(e->pos());
    if (handle != -1) {
        QColor current = points.at(handle).second;
        QColor newColor = QColorDialog::getColor(current,
                                                 this,
                                                 tr("Pick Color"));
        if (newColor.isValid()) {
            points[handle].second = newColor;
            update();
        }
    }
    else {
        e->ignore();
    }
}


QSize GradientWidget::minimumSizeHint(void) const
{
    int spacing = this->style()->pixelMetric(
                QStyle::PM_LayoutHorizontalSpacing);
    if (spacing == -1) {
        spacing = this->style()->layoutSpacing(
                    QSizePolicy::Frame,
                    QSizePolicy::PushButton,
                    Qt::Horizontal);
    }
    return QSize{ int(handleWidth * 1.5 + spacing), handleHeight * 5 };
}


QSize GradientWidget::sizeHint(void) const
{
    int spacing = this->style()->pixelMetric(
                QStyle::PM_LayoutHorizontalSpacing);
    if (spacing == -1) {
        spacing = this->style()->layoutSpacing(
                    QSizePolicy::Frame,
                    QSizePolicy::PushButton,
                    Qt::Horizontal);
    }
    return QSize{ int(handleWidth * 1.1 + spacing), handleHeight };
}


QRect GradientWidget::getGradientRect(void) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    int spacing = this->style()->pixelMetric(
                QStyle::PM_LayoutHorizontalSpacing);
    if (spacing == -1) {
        spacing = this->style()->layoutSpacing(
                    QSizePolicy::Frame,
                    QSizePolicy::PushButton,
                    Qt::Horizontal);
    }
    top += handleHeight / 2;
    bottom += handleHeight / 2;
    return  QRect{ left, top,
                width() - left - right - handleWidth - spacing,
                height() - bottom - top };
}


QRect GradientWidget::getHandleRect(int index) const
{
    QRect handleArea = getHandleArea();
    float y = handleArea.top() + points.at(index).first * handleArea.height();
    return QRect {
        handleArea.x(), int(y - handleHeight / 2),
        handleWidth, handleHeight
    };
}


QRect GradientWidget::getHandleArea(void) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    top += handleHeight / 2;
    bottom += handleHeight / 2;
    float y = top;
    float x = width() - handleWidth - right;
    return QRect {
        int(x), top, handleWidth, height() - top - bottom
    };
}


int GradientWidget::handleAtPos(QPoint pos) const
{
    for (int i = points.size() - 1; i >= 0; i--) {
        QRect rect = getHandleRect(i);
        if (rect.contains(pos)) {
            return i;
        }
    }
    return -1;
}

float GradientWidget::handleYToGradVal(float y) const
{
    QRect area = getHandleArea();
    return (y - area.top()) / area.height();
}


float GradientWidget::gradValToHandleY(float v) const
{
    QRect area = getHandleArea();
    return area.top() + v * area.height();
}

