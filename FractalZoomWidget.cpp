#include "FractalZoomWidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

SliceGrid::SliceGrid(FractalZoomWidget& owner, int level) :
    owner{ owner },
    level{ level },
    dpp{ owner.getDpp(level) }
{
}


std::pair<GridIndex, GridIndex> SliceGrid::getCellIndices(mnd::Real x, mnd::Real y)
{
    return {
        GridIndex(mnd::floor(x / dpp / FractalZoomWidget::chunkSize)),
        GridIndex(mnd::floor(y / dpp / FractalZoomWidget::chunkSize))
    };
}


std::pair<mnd::Real, mnd::Real> SliceGrid::getPositions(GridIndex x, GridIndex y)
{
    return {
        mnd::Real(x) * dpp * FractalZoomWidget::chunkSize,
        mnd::Real(y) * dpp * FractalZoomWidget::chunkSize
    };
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


void SliceGrid::setCell(GridIndex i, GridIndex j, std::unique_ptr<GridElement> tex)
{
    cells[{i, j}] = std::move(tex);
}


void SliceGrid::clearCells(void)
{
    cells.clear();
}


void SliceGrid::clearUncleanCells(void)
{
    for (auto it = cells.begin(); it != cells.end();) {
        if (it->second->img->getRecalcPriority() > 1)
            cells.erase(it++);
        else ++it;
    }
}


Calcer::Calcer(FractalZoomWidget& owner) :
    jobsMutex{ QMutex::Recursive },
    threadPool{ new QThreadPool(this) },
    owner{ owner }
{
    threadPool->setMaxThreadCount(1);
}

void Calcer::clearAll(void)
{
    this->threadPool->clear();
}


void Calcer::calc(SliceGrid& grid, int level, GridIndex i, GridIndex j, int priority)
{
    jobsMutex.lock();
    if (jobs.find({ level, i, j }) == jobs.end()) {
        CalcJob* job = new CalcJob(owner, owner.getGenerator(), &grid, level, i, j, calcState);
        connect(job, &CalcJob::done, this, &Calcer::redirect);
        connect(job, &CalcJob::failed, this, &Calcer::jobFailed);
        connect(job, &QObject::destroyed, this, [this, level, i, j] () { this->notFinished(level, i, j); });
        jobs.emplace(std::tuple{ level, i, j }, job);
        threadPool->start(job, priority);
    }
    jobsMutex.unlock();
}


void Calcer::setCurrentLevel(int level)
{
    if (this->currentLevel != level) {
        this->currentLevel = level;
        std::vector<QRunnable*> toCancel;
        jobsMutex.lock();
        for (auto&[tup, job] : jobs) {
            auto& [level, i, j] = tup;
            if(level != currentLevel) {
                toCancel.push_back(job);
            }
        }
        jobsMutex.unlock();
        for (auto* job : toCancel) {
            if (threadPool->tryTake(job)) {
                delete job;
            }
        }
    }
}


void Calcer::notFinished(int level, GridIndex i, GridIndex j)
{
    jobsMutex.lock();
    jobs.erase({ level, i, j });
    jobsMutex.unlock();
}


void Calcer::jobFailed(int level, GridIndex i, GridIndex j)
{
    jobsMutex.lock();
    jobs.erase({ level, i, j });
    jobsMutex.unlock();
}


void Calcer::redirect(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp)
{
    jobsMutex.lock();
    jobs.erase({ level, i, j });
    jobsMutex.unlock();
    if (this->calcState == calcState) { // TODO remove invalid results correctly
        emit done(level, i, j, bmp);
    }
    else {
        delete bmp;
    }
}


const int FractalZoomWidget::chunkSize = 256;


FractalZoomWidget::FractalZoomWidget(QWidget* parent) :
    EscapeTimeVisualWidget{ parent },
    calcer{ *this }
{
    qMetaTypeId<GridIndex>();
    connect(&calcer, &Calcer::done, this, &FractalZoomWidget::cellReady);
    setMaxIterations(250);
}


void FractalZoomWidget::setViewport(const mnd::MandelViewport& viewport)
{
    mandelInfo.view = viewport;
    mandelInfo.view.adjustAspectRatio(getResolutionX(), getResolutionY());
    update();
}


const mnd::MandelViewport& FractalZoomWidget::getViewport(void) const
{
    return mandelInfo.view;
}


int FractalZoomWidget::getLevel(const mnd::Real& dpp) const
{
    int exponent = (dpp / chunkSize).backend().exponent();
    // replace log2 with cheaper operation
    //return int(mnd::log2(dpp / chunkSize));
    return exponent + 1;
}


mnd::Real FractalZoomWidget::getDpp(int level) const
{
    mnd::Real a = 1;
    a.backend().exponent() += level;
    return a * chunkSize;
    // return mnd::pow(mnd::Real(2), mnd::Real(level)) * chunkSize;
}


SliceGrid& FractalZoomWidget::getGrid(int level)
{
    auto it = levels.find(level);
    if (it != levels.end()) {
        return it->second;
    }
    else {
        levels.insert(std::pair<int, SliceGrid>{ level, SliceGrid{ *this, level } });
        return levels.at(level);
    }
}


void FractalZoomWidget::clearCells(void)
{
    for(auto& [level, grid] : this->levels) {
        grid.clearCells();
    }
}


void FractalZoomWidget::garbageCollect(int level, const GridIndex& /*i*/, const GridIndex& /*j*/)
{
    for(auto& [l, grid] : levels) {
        int dist = ::abs(l - level);

        if (dist == 1) {
            grid.clearUncleanCells();
        }

        if (dist > 20) {
            grid.clearCells();
        }
        else if (dist > 10) {
            if (grid.countAllocatedCells() > 50)
                grid.clearCells();
        }
        else if (dist > 3) {
            if (grid.countAllocatedCells() > 150)
                grid.clearCells();
        }
        else if (dist > 0) {
            if (grid.countAllocatedCells() > 350)
                grid.clearCells();
        }
        else {
            if (grid.countAllocatedCells() > 2500)
                grid.clearCells();
        }
    }
}


GridElement* FractalZoomWidget::searchAbove(int level,
        const GridIndex &i, const GridIndex &j, int recursionLevel)
{
    auto& grid = getGrid(level);
    auto& gridAbove = getGrid(level + 1);

    GridIndex ai = (i < 0 ? (i - 1) : i) / 2;
    GridIndex aj = (j < 0 ? (j - 1) : j) / 2;

    GridElement* above = gridAbove.getCell(ai, aj);

    if (above == nullptr && recursionLevel > 0) {
        auto abFound = searchAbove(level + 1, ai, aj, recursionLevel - 1);
        if (abFound)
            above = abFound;
    }

    if (above != nullptr) {
        auto newElement = std::make_unique<GridElement>(
            false, above->img->clip(short(i & 1), short(j & 1))
        );
        GridElement* ret = newElement.get();
        grid.setCell(i, j, std::move(newElement));
        return ret;
    }
    else {
        return nullptr;
    }
}


GridElement* FractalZoomWidget::searchUnder(int level,
        const GridIndex &i, const GridIndex &j, int recursionLevel)
{
    if (recursionLevel == 0)
        return nullptr;

    auto& grid = getGrid(level);
    auto& gridUnder = getGrid(level - 1);

    GridIndex ai = i * 2;
    GridIndex aj = j * 2;

    GridElement* u00 = gridUnder.getCell(ai, aj);
    GridElement* u01 = gridUnder.getCell(ai, aj + 1);
    GridElement* u10 = gridUnder.getCell(ai + 1, aj);
    GridElement* u11 = gridUnder.getCell(ai + 1, aj + 1);

    /*if (   u00 == nullptr
        || u01 == nullptr
        || u10 == nullptr
        || u11 == nullptr) {
        auto abFound = searchUnder(level + 1, ai, aj, recursionLevel - 1);
        if (abFound)
            above = abFound;
    }*/

    if (   u00 != nullptr
        && u01 != nullptr
        && u10 != nullptr
        && u11 != nullptr) {
        auto newElement = std::make_unique<GridElement>(
            false, std::make_shared<QuadImage>(u00->img, u01->img, u10->img, u11->img)
        );
        GridElement* ret = newElement.get();
        grid.setCell(i, j, std::move(newElement));
        return ret;
    }
    else {
        return nullptr;
    }
}


const mnd::MandelInfo& FractalZoomWidget::getMandelInfo(void) const
{
    return mandelInfo;
}


void FractalZoomWidget::setGenerator(mnd::MandelGenerator* gen)
{
    bool changed = true;
    if (this->generator == gen)
        changed = false;

    this->generator = gen;

    if (changed) {
        clearCells();
        update();
    }
}


mnd::MandelGenerator* FractalZoomWidget::getGenerator(void) const
{
    return generator;
}


void FractalZoomWidget::zoom(float factor)
{
    mandelInfo.view.zoomCenter(factor);
    update();
}


void FractalZoomWidget::setSmoothColoring(bool smooth)
{
    if (mandelInfo.smooth != smooth) {
        mandelInfo.smooth = smooth;
        clearCells();
        update();
    }
}


void FractalZoomWidget::setMaxIterations(int maxIterations)
{
    if (mandelInfo.maxIter != maxIterations) {
        mandelInfo.maxIter = maxIterations;
        EscapeTimeVisualWidget::setMaxIterationCutoff(float(maxIterations));
        clearCells();
        update();
    }
}


void FractalZoomWidget::initializeGL(void)
{
    EscapeTimeVisualWidget::initializeGL();
    Bitmap<float> empty{ 1, 1 };
    empty.get(0, 0) = 0.0f;
    emptyImage = new ETVImage(*this, empty);
}


void FractalZoomWidget::resizeGL(int w, int h)
{
    EscapeTimeVisualWidget::resizeGL(w, h);
    mandelInfo.view.height = mandelInfo.view.width * h / w;
}


void FractalZoomWidget::paintGL(void)
{
    EscapeTimeVisualWidget::paintGL();
    /*ETVImage etvi{ *this };

    auto& gl = *this->context()->functions();
    gl.glClearColor(0.0, 0.2, 0.0, 1.0);
    gl.glClear(GL_COLOR_BUFFER_BIT);

    etvi.draw(100, 100, 700, 700);*/


    ////////////////////

    int width = getResolutionX();
    int height = getResolutionY();

    auto& mvp = mandelInfo.view;
    mnd::Real dpp = mvp.width / width;
    int level = getLevel(dpp) - 1;
    auto& grid = getGrid(level);
    mnd::Real gw = getDpp(level) * chunkSize;
    auto [left, top] = grid.getCellIndices(mvp.x, mvp.y);
    auto [right, bottom] = grid.getCellIndices(mvp.right(), mvp.bottom());

    garbageCollect(level, (left + right) / 2, (top + bottom) / 2);
    emit calcer.setCurrentLevel(level);

    mnd::Real w = width * gw / mvp.width;

    auto [realXLeft, realYTop] = grid.getPositions(left, top);
    realXLeft = ((realXLeft - mvp.x) * mnd::Real(width)) / mvp.width;
    realYTop = ((realYTop - mvp.y) * mnd::Real(height)) / mvp.height;
    for(GridIndex i = left; i <= right; i++) {
        for(GridIndex j = top; j <= bottom; j++) {
            mnd::Real x = w * int(i - left) + realXLeft;
            mnd::Real y = w * int(j - top) + realYTop;

            GridElement* t = grid.getCell(i, j);

            if (t == nullptr) {
                auto under = searchUnder(level, i, j, 1);
                if (under) {
                    t = under;
                }
                else {
                    auto above = searchAbove(level, i, j, 3);
                    if (above) {
                        t = above;
                    }
                }
            }

            if (t != nullptr) {
                if (!t->enoughResolution) {
                    auto under = searchUnder(level, i, j, 1);
                    if (under) {
                        t = under;
                    }
                    calcer.calc(grid, level, i, j, t->img->getRecalcPriority());
                }

                t->img->drawRect(float(x), float(y), float(w), float(w));
            }
            else {
                calcer.calc(grid, level, i, j, 1000);
                if (emptyImage)
                    emptyImage->draw(float(x), float(y), float(w), float(w));
            }
        }
    }
}


void FractalZoomWidget::cellReady(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp)
{
    this->getGrid(level).setCell(i, j,
        std::make_unique<GridElement>(true, std::make_shared<ImageClip>(std::make_shared<ETVImage>(*this, *bmp))));
    delete bmp;
    update();
}
