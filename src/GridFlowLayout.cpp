#include "GridFlowLayout.h"
#include <QWidget>
#include <QStyle>

GridFlowLayout::GridFlowLayout(QWidget* parent) :
    QLayout{ parent }
{
}


void GridFlowLayout::addItem(QLayoutItem* item)
{
    items.append(item);
}


QLayoutItem* GridFlowLayout::itemAt(int index) const
{
    return items.value(index);
}


QLayoutItem* GridFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < items.size())
        return items.takeAt(index);
    return nullptr;
}


bool GridFlowLayout::hasHeightForWidth(void) const
{
    return true;
}


int GridFlowLayout::heightForWidth(int width) const
{
    return 300;
}


int GridFlowLayout::count(void) const
{
    return items.size();
}


Qt::Orientations GridFlowLayout::expandingDirections(void) const
{
    return Qt::Orientations();
}


QSize GridFlowLayout::minimumSize(void) const
{
    return QSize{ 400, 400 };
}


QSize GridFlowLayout::sizeHint(void) const
{
    return minimumSize();
}


void GridFlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    int mleft, mtop, mright, mbottom;
    getContentsMargins(&mleft, &mtop, &mright, &mbottom);
    QRect area = rect.adjusted(mleft, mtop, -mright, -mbottom);

    QVector<QSize> sizeHints;
    for (QLayoutItem* item : qAsConst(items)) {
        sizeHints.append(item->sizeHint());
    }

    int rows;
    for (rows = count(); rows > 1; rows--) {
        if (checkRows(rows, rect, sizeHints)) {
            break;
        }
    }
    rows = 1;
    applyLayout(rows, sizeHints, area);

/*
    int x = area.x();
    int y = area.y();
    for (QLayoutItem* item : qAsConst(items)) {
        const QWidget* widget = item->widget();
        int spaceX = 5;
        int spaceY = 5;
        int width = area.width();
        int height = 200;//area.height();
        item->setGeometry(QRect(x, y, width, height));
        y += height;
    }*/
}


void GridFlowLayout::applyLayout(int rows, const QVector<QSize>& sizeHints,
                                 const QRect& area)
{
    calculateRows(rows, sizeHints);
    int x = area.x();
    for (int i = 0; i < rows; i++) {
        int width = widths[i];
        int y = area.y();
        for (int j = 0; j <= count() / rows; j++) {
            int index = i + j * rows;
            if (index < count()) {
                int height = heights[j];
                QLayoutItem* item = items.value(index);
                const QWidget* widget = item->widget();
                item->setGeometry(QRect(x, y, width, height));
                y += height;
            }
        }
        x += width;
    }
}


void GridFlowLayout::calculateRows(int rows, const QVector<QSize>& sizeHints)
{
    widths.clear();
    heights.clear();
    for (int i = 0; i < rows; i++) {
        int maxWidth = 0;
        for(int j = i; j < sizeHints.size(); j += rows) {
            maxWidth = std::max(maxWidth, sizeHints[j].width());
        }
        widths.append(maxWidth);
    }
    for (int i = 0; i < sizeHints.size(); i += rows) {
        int maxHeight = 0;
        for(int j = i; j < i + rows && j < sizeHints.size(); j++) {
            maxHeight = std::max(maxHeight, sizeHints[j].height());
        }
        heights.append(maxHeight);
    }
}


bool GridFlowLayout::checkRows(int rows, const QRect& rect,
                               const QVector<QSize>& sizeHints)
{
    calculateRows(rows, sizeHints);
    int width = 0;
    int height = 0;
    for (int w : widths) width += w;
    for (int h : heights) height += h;

    return width <= rect.width() && height <= rect.height();
}
