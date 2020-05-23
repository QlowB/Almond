#include "FractalZoomWidget.h"


SliceGrid::SliceGrid(FractalZoomWidget& owner, int level) :
    owner{ owner },
    level{ level },
    dpp{ owner.getDpp(level) }
{
}


std::pair<GridIndex, GridIndex> SliceGrid::getCellIndices(mnd::Real x, mnd::Real y)
{
    return { GridIndex(mnd::floor(x / dpp / FractalZoomWidget::chunkSize)), GridIndex(mnd::floor(y / dpp / MandelView::chunkSize)) };
}


std::pair<mnd::Real, mnd::Real> SliceGrid::getPositions(GridIndex x, GridIndex y)
{
    return { mnd::Real(x) * dpp * FractalZoomWidget::chunkSize, mnd::Real(y) * dpp * MandelView::chunkSize };
}


GridElement* SliceGrid::getCell(GridIndex i, GridIndex j)
{
    auto cIt = cells.find({i, j});
    if (cIt != cells.end()) {
        return cIt->second.get();
    }
    else {
        return nullptr;
    }
}


void TexGrid::setCell(GridIndex i, GridIndex j, std::unique_ptr<GridElement> tex)
{
    cells[{i, j}] = std::move(tex);
}


void TexGrid::clearCells(void)
{
    cells.clear();
}


void TexGrid::clearUncleanCells(void)
{
    for (auto it = cells.begin(); it != cells.end();) {
        if (it->second->img->getRecalcPriority() > 1)
            cells.erase(it++);
        else ++it;
    }
}


FractalZoomWidget::FractalZoomWidget(QWidget* parent) :
    EscapeTimeVisualWidget{ parent }
{
}



