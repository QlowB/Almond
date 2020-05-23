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


void Calcer::jobFailed(void)
{

}


void Calcer::redirect(Bitmap<float>* bmp)
{
    QObject* s = sender();
    if (CalcJob* cj = qobject_cast<CalcJob*>(s)) {
        int level = cj->level;
        GridIndex i = cj->i;
        GridIndex j = cj->j;
        jobsMutex.lock();
        jobs.erase({ level, i, j });
        jobsMutex.unlock();
        if (this->calcState == calcState) {
            emit done(level, i, j, bmp);
        }
        else {
            delete bmp;
        }
    }
    else {
        delete bmp;
    }
}


FractalZoomWidget::FractalZoomWidget(QWidget* parent) :
    EscapeTimeVisualWidget{ parent },
    calcer{ *this }
{
}


mnd::Real FractalZoomWidget::getDpp(int level)
{
    return mnd::pow(mnd::Real(2), mnd::Real(level)) * chunkSize;
}


const mnd::MandelInfo& FractalZoomWidget::getMandelInfo(void) const
{
    return mandelInfo;
}


void FractalZoomWidget::setGenerator(mnd::MandelGenerator* gen)
{
    this->generator = gen;
}


mnd::MandelGenerator* FractalZoomWidget::getGenerator(void) const
{
    return generator;
}


void FractalZoomWidget::paintGL(void)
{
    ETVImage etvi{ *this };

    auto& gl = *this->context()->functions();
    gl.glClearColor(0.0, 0.2, 0.0, 1.0);
    gl.glClear(GL_COLOR_BUFFER_BIT);

    etvi.draw(100, 100, 700, 700);
}
