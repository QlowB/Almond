#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H

#include <QWidget>
#include <QLinearGradient>

#include <utility>

class GradientWidget :
    public QWidget
{
    Q_OBJECT
    QVector<std::pair<float, QColor>> points;

    bool dragging;
    int selectedHandle;
    float selectOffsetY;

    int mouseOver;

    int handleWidth = 60;
    int handleHeight = 30;
public:
    explicit GradientWidget(QWidget *parent = nullptr);

    const QVector<std::pair<float, QColor>>& getGradient(void) const;
    void setGradient(QVector<std::pair<float, QColor>>);

    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

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
signals:

};

#endif // GRADIENTWIDGET_H

