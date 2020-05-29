#ifndef GRIDFLOWLAYOUT_H
#define GRIDFLOWLAYOUT_H

#include <QLayout>
#include <QList>
#include <QVector>

class GridFlowLayout : 
    public QLayout
{
    QList<QLayoutItem*> items;
    QVector<int> widths;
    QVector<int> heights;
public:
    GridFlowLayout(QWidget* parent);

    void                addItem             (QLayoutItem* item) override;
    QLayoutItem*        itemAt              (int index) const override;
    QLayoutItem*        takeAt              (int index) override;
    bool                hasHeightForWidth   (void) const override;
    int                 heightForWidth      (int width) const override;
    int                 count               (void) const override;
    Qt::Orientations    expandingDirections (void) const override;
    QSize               minimumSize         (void) const override;
    QSize               sizeHint            (void) const override;
    void                setGeometry         (const QRect& rect) override;
private:
    void                applyLayout         (int rows, const QVector<QSize>& sizeHints,
                                             const QRect& area);
    void                calculateRows       (int rows, const QVector<QSize>& sizeHints);
    bool                checkRows           (int rows, const QRect& rect,
                                             const QVector<QSize>& sizeHints);
    
};


#endif // GRIDFLOWLAYOUT_H

