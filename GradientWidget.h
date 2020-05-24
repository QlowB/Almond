#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H

#include <QWidget>
#include <QLinearGradient>
#include <QPainterPath>
#include <QVector>
#include <QPair>

class GradientWidget :
    public QWidget
{
    Q_OBJECT
    QVector<QPair<float, QColor>> points;

    bool dragging;
    int selectedHandle;
    float selectOffsetY;

    int mouseOver;

    int handleWidth = 40;
    int handleHeight = 24;
public:

    enum HandleState
    {
        HANDLE_NORMAL = 0x00,
        HANDLE_MOUSEOVER = 0x01,
        HANDLE_DOWN = 0x02,
        HANDLE_SELECTED = 0x04
    };


    explicit GradientWidget(QWidget *parent = nullptr);

    const QVector<QPair<float, QColor>>& getGradient(void) const;
    void setGradient(QVector<QPair<float, QColor>>);

    QColor colorAtY(float y);

    virtual void paintEvent(QPaintEvent* e) override;
    virtual void paintHandle(QPainter& painter, const QRectF &pos, QColor c, int handleState);

    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* e) override;

    QSize minimumSizeHint(void) const override;
    QSize sizeHint(void) const override;

protected:
    /// \brief the area in which the gradient is displayed
    QRect getGradientRect(void) const;

    /// \brief the area in which the handle with index
    /// \c index is displayed
    QRect getHandleRect(int index) const;

    /// \brief the area in which the handles can move around
    QRect getHandleArea(void) const;

    int handleAtPos(QPoint pos) const;

    float handleYToGradVal(float y) const;
    float gradValToHandleY(float v) const;

private:
    static QPainterPath createSlideHandle(float w, float h);
signals:
    void gradientChanged(void);
};

#endif // GRADIENTWIDGET_H

