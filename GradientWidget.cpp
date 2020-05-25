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

    maxValue = 1.0f;

    colorPicker = new QColorDialog(this);
    colorPicker->setOption(QColorDialog::NoButtons);
    connect(colorPicker, &QColorDialog::currentColorChanged, this, &GradientWidget::selectedColorChanged);

    setMouseTracking(true);
}


/*const QVector<QPair<float, QColor>>& GradientWidget::getGradient(void) const
{
    return points;
}*/

const Gradient& GradientWidget::getGradient(void) const
{
    return gradient;
}


void GradientWidget::setGradient(Gradient gr)
{
    gradient = std::move(gr);
    points = gradient.getPoints();
    maxValue = gradient.getMax();
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


void GradientWidget::updateGradient(void)
{
    gradient = Gradient{ points, maxValue };
}


QColor GradientWidget::colorAtY(float y)
{
    float v = handleYToGradVal(y);
    QColor up = QColor(QColor::Invalid);
    QColor down = QColor(QColor::Invalid);
    float upv = 0;
    float downv = 1;
    for (const auto& [color, val] : points) {
        if (val >= upv && val < v) {
            upv = val;
            up = QColor(color.r, color.g, color.b);
        }
        if (val <= downv && val > v) {
            downv = val;
            down = QColor(color.r, color.g, color.b);
        }
    }

    if (!up.isValid())
        return down;
    if (!down.isValid())
        return up;
    return lerp(up, down, (v - upv) / (downv - upv));
}


void GradientWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter{ this };

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

    QLinearGradient linGrad;
    linGrad.setStart(0, gradientRect.top());
    linGrad.setFinalStop(0, gradientRect.bottom());

    const int stops = this->height() / 5;
    /*for (int i = 0; i < stops; i++) {
        auto col = gradient.get(float(i) / stops * gradient.getMax());
        linGrad.setColorAt(float(i) / stops, QColor{ col.r, col.g, col.b });
    }*/

    for (const auto& [col, at] : points) {
        linGrad.setColorAt(at / maxValue, QColor{ col.r, col.g, col.b });
    }

    // adjust rect to have small margins, so the frame
    // around the gradient is visible
    gradientRect.adjust(fhmargins, fvmargins, -fhmargins, -fvmargins);
    float lastPoint = 0;
    QColor lastColor = QColor{ 0, 0, 0 };



    /*std::vector<int> orderedIndices(points.size());
    for (int i = 0; i < points.size(); i++)
        orderedIndices.push_back(i);

    std::sort(orderedIndices.begin(), orderedIndices.end(),
        [this] (int l, int r) {
        return points[l].first < points[r].first;
    });

    // traverse gradient in order and interpolate in linear
    // RGB space to avoid interpolating in sRGB
    for (int i = 0; i < orderedIndices.size(); i++) {
        int index = orderedIndices[i];
        auto& [point, color] = points[index];
        int m = 17;
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
    }*/

    QBrush brush{ linGrad };
    painter.fillRect(gradientRect, brush);

    int index = 0;
    for (auto& [color, point] : points) {
        QRect r = getHandleRect(index);
        /*QStyleOptionButton so;
        so.init(this);
        so.rect = r;
        if (dragging && selectedHandle == index)
            so.state |= QStyle::State_Sunken;
        else if (selectedHandle == index)
            so.state |= QStyle::State_HasFocus;
        else
            so.state &= ~QStyle::State_Sunken & ~QStyle::State_HasFocus;
        if (mouseOver == index)
            so.state |= QStyle::State_MouseOver;
        else
            so.state &= ~QStyle::State_MouseOver;
        
        so.palette.setColor(QPalette::ColorRole::Button, color);
        style()->drawControl(QStyle::ControlElement::CE_PushButton, &so, &painter, this);*/
        int hs = HandleState::HANDLE_NORMAL;
        if (dragging && selectedHandle == index)
            hs |= HANDLE_DOWN;
        if (mouseOver == index)
            hs |= HANDLE_MOUSEOVER;
        if (selectedHandle == index)
            hs |= HANDLE_SELECTED;
        paintHandle(painter, r, fromRGB(color), hs);
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


void GradientWidget::paintHandle(QPainter& painter, const QRectF& pos,
                                 QColor c, int handleState)
{
    const float lineWidth = 2;
    QPainterPath qpp = createSlideHandle(pos.width() - lineWidth, pos.height() - lineWidth);
    qpp.translate(pos.x() + lineWidth / 2, pos.y() + lineWidth / 2);
    if (handleState & HANDLE_SELECTED) {
        QColor absLighter;
        absLighter.setHsvF(c.hueF(), c.saturationF(), c.valueF() > 0.3 ? c.valueF() : 0.3);
        painter.setPen(QPen(QBrush(absLighter.lighter(130)), lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else
        painter.setPen(QPen(QBrush(c.darker(200)), lineWidth / 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient bevel{ 0, pos.top(), 0, pos.bottom() }; // top down linear gradient

    if (handleState & HANDLE_DOWN) {
        bevel.setColorAt(0, c.darker(120));
        bevel.setColorAt(1, c.lighter(120));
    }
    else if (handleState & HANDLE_MOUSEOVER) {
        bevel.setColorAt(0, c.lighter(130));
        bevel.setColorAt(1, c.darker(110));
    }
    else {
        bevel.setColorAt(0, c.lighter(120));
        bevel.setColorAt(1, c.darker(120));
    }
    painter.fillPath(qpp, QBrush(bevel));
    painter.drawPath(qpp);
}


void GradientWidget::mousePressEvent(QMouseEvent* e)
{
    int handle = handleAtPos(e->pos());

    if (handle != -1) {
        selectedHandle = handle;
        dragging = true;
        selectOffsetY = e->y() - gradValToHandleY(
                    points[handle].second);
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
        newVal = std::clamp(newVal, 0.0f, maxValue);
        points[selectedHandle].second = newVal;
        updateGradient();
        update();
        emit gradientChanged();
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
    auto torgb = [](const QColor& c) {
        return RGBColor{
            uint8_t(c.red()), uint8_t(c.green()), uint8_t(c.blue())
        };
    };
    QRect handleArea = getHandleArea();
    int handle = handleAtPos(e->pos());
    if (handle != -1) {
        RGBColor current = points.at(handle).first;
        /*QColor newColor = QColorDialog::getColor(current,
                                                 this,
                                                 tr("Pick Color"));*/
        selectedHandle = handle;
        colorPicker->setCurrentColor(fromRGB(current));
        colorPicker->exec();
        /*if (newColor.isValid()) {
            points[handle].first = torgb(newColor);
            update();
            emit gradientChanged();
        }*/
        e->accept();
    }
    else if (handleArea.contains(e->pos())) {
        float v = handleYToGradVal(e->pos().y());
        points.emplace_back(torgb(colorAtY(e->pos().y())), v);
        e->accept();
        updateGradient();
        update();
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


void GradientWidget::selectedColorChanged(const QColor& newColor)
{
    if (points.size() > selectedHandle) {
        points.at(selectedHandle).first = RGBColor {
            uint8_t(newColor.red()),
            uint8_t(newColor.green()),
            uint8_t(newColor.blue())
        };
        updateGradient();
        update();
        emit gradientChanged();
    }
}


QRect GradientWidget::getGradientRect(void) const
{
    QMargins cm = contentsMargins();
    int top = cm.top();
    int bottom = cm.bottom();
    int left = cm.left();
    int right = cm.right();

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
    float y = handleArea.top() + points.at(index).second / maxValue * handleArea.height();
    return QRect {
        handleArea.x(), int(y - handleHeight / 2),
        handleWidth, handleHeight
    };
}


QRect GradientWidget::getHandleArea(void) const
{
    QMargins cm = contentsMargins();
    int top = cm.top();
    int bottom = cm.bottom();
    int left = cm.left();
    int right = cm.right();

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
    return maxValue * (y - area.top()) / area.height();
}


float GradientWidget::gradValToHandleY(float v) const
{
    QRect area = getHandleArea();
    return area.top() + v / maxValue * area.height();
}


QPainterPath GradientWidget::createSlideHandle(float w, float h)
{
    const float rounding = 4;
    QPainterPath qpp;
    QPolygonF qpf;
    qpf << QPointF{ 0, 0.5 * h }
        << QPointF{ 0.3 * w, h };
    qpp.moveTo(0, 0.5 * h);
    qpp.lineTo(0.3 * w, h);
    qpp.arcTo(w - rounding, h - rounding, rounding, rounding, -90, 90);
    qpp.arcTo(w - rounding, 0, rounding, rounding, 0, 90);
    qpp.lineTo(0.3 * w, 0);
    qpp.lineTo(0, 0.5 * h);

    return qpp;
}

