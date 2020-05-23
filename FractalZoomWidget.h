#ifndef FRACTALZOOMWIDGET_H
#define FRACTALZOOMWIDGET_H

#include "EscapeTimeVisualWidget.h"
#include "FractalWidgetUtils.h"

class FractalZoomWidget;


///
/// \brief represents a grid of images at a certain depth
///        in the fractal.
///
class SliceGrid
{
public:
    FractalZoomWidget& owner;
    int level;
    mnd::Real dpp;
    std::unordered_map<std::pair<GridIndex, GridIndex>, std::unique_ptr<GridElement>, IndexPairHash> cells;
public:
    SliceGrid(FractalZoomWidget& owner, int level);

    std::pair<GridIndex, GridIndex> getCellIndices(mnd::Real x, mnd::Real y);
    std::pair<mnd::Real, mnd::Real> getPositions(GridIndex i, GridIndex j);
    GridElement* getCell(GridIndex i, GridIndex j);
    void setCell(GridIndex i, GridIndex j, std::unique_ptr<GridElement> tex);

    inline size_t countAllocatedCells(void) const { return cells.size(); }
    void clearCells(void);
    void clearUncleanCells(void);
};


class FractalZoomWidget :
    public EscapeTimeVisualWidget
{
    Q_OBJECT

    mnd::MandelGenerator* generator;

public:
    static const int chunkSize = 256;

    FractalZoomWidget(QWidget* parent = nullptr);
};

#endif // FRACTALZOOMWIDGET_H
