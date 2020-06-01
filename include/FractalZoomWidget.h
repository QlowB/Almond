#ifndef FRACTALZOOMWIDGET_H
#define FRACTALZOOMWIDGET_H

#include "EscapeTimeVisualWidget.h"
#include "FractalWidgetUtils.h"
#include "Bitmap.h"

#include <QThreadPool>
#include <QMutex>
#include <QOffscreenSurface>

class FractalZoomWidget;

Q_DECLARE_METATYPE(std::shared_ptr<ETVImage>)


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


class Calcer : public QObject
{
    Q_OBJECT

    /// tuple contains level, i, j of the job
    std::unordered_map<std::tuple<int, GridIndex, GridIndex>, CalcJob*, IndexTripleHash> jobs;
    QMutex jobsMutex;
    QThreadPool* threadPool;
    FractalZoomWidget& owner;
    int currentLevel;

    volatile unsigned int calcState = 0;
public:
    Calcer(FractalZoomWidget& owner);
    void clearAll(void);

    inline void changeState(void) { calcState++; }

public slots:
    void calc(SliceGrid& grid, int level, GridIndex i, GridIndex j, int priority);
    void setCurrentLevel(int level);
    void notFinished(int level, GridIndex i, GridIndex j);
    void jobFailed(int level, GridIndex i, GridIndex j);
    void redirect(int level, GridIndex i, GridIndex j, int calcState, Bitmap<float>* bmp);
signals:
    void done(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp);
};


class TextureUploader :
    public QThread
{
    Q_OBJECT

    QOpenGLContext* context;
    QOffscreenSurface* surface;
    EscapeTimeVisualWidget& owner;
public:
    TextureUploader(EscapeTimeVisualWidget& shareWith, QObject* parent = nullptr);

public slots:
    void upload(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp);
signals:
    void uploaded(int level, GridIndex i, GridIndex j, std::shared_ptr<ETVImage>);
};


class FractalZoomWidget :
    public EscapeTimeVisualWidget
{
    Q_OBJECT

    // a grid should not be deleted once constructed.
    // to free up memory one can call SliceGrid::clearCells()
    std::unordered_map<int, SliceGrid> levels;
    mnd::MandelGenerator* generator;
    Calcer calcer;

    std::unique_ptr<ETVImage> emptyImage;

    const bool useUploadThread = false;
    TextureUploader* uploader;
    QThread* uploadeThread;

protected:
    mnd::MandelInfo mandelInfo;

public:
    static const int chunkSize;

    FractalZoomWidget(QWidget* parent = nullptr);

    Q_PROPERTY(mnd::MandelViewport viewport READ getViewport WRITE setViewport)
    virtual void setViewport(const mnd::MandelViewport& viewport);
    virtual const mnd::MandelViewport& getViewport(void) const;

    int getLevel(const mnd::Real& dpp) const;
    mnd::Real getDpp(int level) const;
    SliceGrid& getGrid(int level);

    void clearCells(void);

    void garbageCollect(int level,
         const GridIndex& i, const GridIndex& j);

    GridElement* searchAbove(int level,
                 const GridIndex& i, const GridIndex& j,
                 int recursionLevel);
    GridElement* searchUnder(int level,
                 const GridIndex& i, const GridIndex& j,
                 int recursionLevel);

    const mnd::MandelInfo& getMandelInfo(void) const;
    mnd::MandelInfo& getMandelInfo(void);

    void setGenerator(mnd::MandelGenerator*);
    mnd::MandelGenerator* getGenerator(void) const;

    virtual void zoom(float factor);

    virtual void setSmoothColoring(bool smooth);
    virtual void setMaxIterations(int maxIterations);

    virtual void initializeGL(void) override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL(void) override;

protected slots:
    void cellReady(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp);
    void cellReadyTex(int level, GridIndex i, GridIndex j, std::shared_ptr<ETVImage> img);
};

#endif // FRACTALZOOMWIDGET_H
