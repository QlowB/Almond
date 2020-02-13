#pragma once

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QThreadPool>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMutex>
//#include <qopengl.h>
//#include <qopenglfunctions.h>
//#include <qopenglcontext.h>

#include "Bitmap.h"
#include "Gradient.h"
#include <Mandel.h>

#include <atomic>
#include <tuple>
#include <deque>
#include <chrono>
#include <unordered_map>
#include <unordered_set>


using GridIndex = mnd::Integer;
Q_DECLARE_METATYPE(GridIndex)


class MandelView;
class MandelWidget;

class Texture
{
    GLuint id;
public:
    Texture(const Bitmap<RGBColor>& pict, GLint param = GL_LINEAR);
    ~Texture(void);

    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture(Texture&& other);
    Texture& operator=(Texture&& other);

private:
    void bind(void) const;
public:
    inline GLuint getId(void) const { return id; }

    void drawRect(float x, float y, float width, float height);
};


class CellImage
{
public:
    CellImage(void) = default;
    CellImage(CellImage&& b) = default;
    CellImage(const CellImage& b) = default;
    CellImage& operator=(const CellImage& b) = default;
    CellImage& operator=(CellImage&& b) = default;
    virtual ~CellImage(void);

    virtual void drawRect(float x, float y, float width, float height) = 0;
    virtual std::shared_ptr<CellImage> clip(short i, short j) = 0;
    virtual int getRecalcPriority(void) const = 0;
};


class TextureClip : public CellImage
{
    std::shared_ptr<Texture> texture;
    float tx, ty, tw, th;
public:
    inline TextureClip(std::shared_ptr<Texture> tex,
                       float tx, float ty, float tw, float th) :
        texture{ std::move(tex) },
        tx{ tx }, ty{ ty }, tw{ tw }, th{ th }
    {}

    inline TextureClip(std::shared_ptr<Texture> tex) :
        TextureClip{ tex, 0.0f, 0.0f, 1.0f, 1.0f }
    {}

    virtual ~TextureClip(void);

    void drawRect(float x, float y, float width, float height);

    TextureClip clip(float x, float y, float w, float h);
    std::shared_ptr<CellImage> clip(short i, short j);
    int getRecalcPriority(void) const;
};


class QuadImage : public CellImage
{
    std::shared_ptr<CellImage> cells[2][2];
public:
    inline QuadImage(std::shared_ptr<CellImage> i00,
                     std::shared_ptr<CellImage> i01,
                     std::shared_ptr<CellImage> i10,
                     std::shared_ptr<CellImage> i11) :
        cells{ { std::move(i00), std::move(i01) }, { std::move(i10), std::move(i11) } }
    {}
    virtual ~QuadImage(void);

    void drawRect(float x, float y, float width, float height);
    std::shared_ptr<CellImage> clip(short i, short j);
    int getRecalcPriority(void) const;
};


struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return (h1 ^ 234579245) * 23452354 + h2;
    }
};

struct TripleHash {
    template <typename T1, typename T2, typename T3>
    std::size_t operator () (const std::tuple<T1, T2, T3>& p) const {
        auto h1 = std::hash<T1>{}(std::get<0>(p));
        auto h2 = std::hash<T2>{}(std::get<1>(p));
        auto h3 = std::hash<T3>{}(std::get<2>(p));
        return (((h1 ^ 234579245) * 23452357 + h2) ^ 2345244345) * 23421 + h3;
    }
};


struct GridElement
{
    bool enoughResolution;
    std::shared_ptr<CellImage> img;
    inline GridElement(bool enoughResolution, std::shared_ptr<CellImage> img) :
        enoughResolution{ enoughResolution },
        img{ std::move(img) }
    {}
};


class TexGrid
{
public:
    MandelView& owner;
    int level;
    mnd::Real dpp;
    std::unordered_map<std::pair<GridIndex, GridIndex>, std::unique_ptr<GridElement>, PairHash> cells;
public:
    //inline TexGrid(MandelV& owner) : level{ 1.0 }, owner{ owner } {}
    TexGrid(MandelView& owner, int level);

    std::pair<GridIndex, GridIndex> getCellIndices(mnd::Real x, mnd::Real y);
    std::pair<mnd::Real, mnd::Real> getPositions(GridIndex i, GridIndex j);
    GridElement* getCell(GridIndex i, GridIndex j);
    void setCell(GridIndex i, GridIndex j, std::unique_ptr<GridElement> tex);

    inline size_t countAllocatedCells(void) const { return cells.size(); }
    void clearCells(void);
};


class Job : public QObject, public QRunnable
{
    Q_OBJECT
public:
    mnd::Generator* generator;
    const Gradient& gradient;
    int maxIter;
    TexGrid* grid;
    int level;
    GridIndex i, j;
    long calcState = 0;

    inline Job( mnd::Generator* generator,
               const Gradient& gradient,
               int maxIter,
               TexGrid* grid,
               int level, GridIndex i, GridIndex j,
               long calcState) :
        generator{ generator },
        gradient{ gradient },
        maxIter{ maxIter },
        grid{ grid },
        level{ level },
        i{ i }, j{ j },
        calcState{ calcState }
    {}

    void run() override;
signals:
    void done(int level, GridIndex i, GridIndex j, long calcState, Bitmap<RGBColor>* bmp);
};


class Calcer : public QObject
{
    Q_OBJECT
    /// tuple contains level, i, j of the job
    std::unordered_map<std::tuple<int, GridIndex, GridIndex>, Job*, TripleHash> jobs;
    QMutex jobsMutex;
    mnd::Generator* generator;
    std::unique_ptr<QThreadPool> threadPool;
    const Gradient& gradient;
    int maxIter;
    int currentLevel;

    volatile long calcState = 0;
public:
    inline Calcer(mnd::Generator* generator, const Gradient& gradient, int maxIter) :
        jobsMutex{ QMutex::Recursive },
        generator{ generator },
        threadPool{ std::make_unique<QThreadPool>() },
        gradient{ gradient },
        maxIter{ maxIter }
    {
        threadPool->setMaxThreadCount(1);
    }

    void setMaxIter(int maxIter);
    void clearAll(void);
    void setGenerator(mnd::Generator* generator) { this->generator = generator; changeState(); }

    inline void changeState(void) { calcState++; }

public slots:
    void calc(TexGrid& grid, int level, GridIndex i, GridIndex j, int priority);
    void setCurrentLevel(int level);
    void notFinished(int level, GridIndex i, GridIndex j);
    void redirect(int level, GridIndex i, GridIndex j, long calcState, Bitmap<RGBColor>* bmp);
signals:
    void done(int level, GridIndex i, GridIndex j, Bitmap<RGBColor>* bmp);
};


class MandelView : public QObject
{
    Q_OBJECT
public:
    std::unique_ptr<Texture> empty;

    // a grid should not be deleted once constructed.
    // to free up memory one can call TexGrid::clearCells()
    std::unordered_map<int, TexGrid> levels;
    mnd::Generator* generator;
    Calcer calcer;
    MandelWidget& owner;
    int maxIter;
    int width;
    int height;
public:
    static const int chunkSize = 256;
    MandelView(mnd::Generator* generator, MandelWidget& owner, int maxIter);
    int getLevel(mnd::Real dpp);
    mnd::Real getDpp(int level);

    TexGrid& getGrid(int level);

    inline int getMaxIter(void) const { return this->maxIter; }
    void setMaxIter(int maxIter);

    void setGenerator(mnd::Generator* generator);

    void clearCells(void);

    void garbageCollect(int level, GridIndex i, GridIndex j);
    GridElement* searchAbove(int level, GridIndex i, GridIndex j, int recursionLevel);
    GridElement* searchUnder(int level, GridIndex i, GridIndex j, int recursionLevel);
    void paint(const mnd::MandelViewport& mvp);
public slots:
    void cellReady(int level, GridIndex i, GridIndex j, Bitmap<RGBColor>* bmp);
signals:
    void redrawRequested(void);
};


class MandelWidget : public QOpenGLWidget
{
    Q_OBJECT
private:
    mnd::MandelContext& mndContext;

    bool smoothColoring = true;

    Gradient gradient;

    bool initialized = false;
    int maxIterations = 2000;


    volatile bool rubberbanding = false;
    QRectF rubberband;
    volatile bool dragging = false;
    int dragX, dragY;

    mnd::MandelViewport currentViewport;
    mnd::MandelViewport targetViewport;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastAnimUpdate;

    std::unique_ptr<MandelView> mandelView;
public:
    MandelWidget(mnd::MandelContext& ctxt, QWidget* parent = nullptr);
    ~MandelWidget(void) override;

    inline const Gradient& getGradient(void) const { return gradient; }
    void setGradient(Gradient g);

    inline bool getSmoothColoring(void) const { return smoothColoring; }
    void setSmoothColoring(bool sc);

    void initializeGL(void) override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

private:
    void updateAnimations(void);

    void drawRubberband(void);
public:

    void zoom(float scale, float x = 0.5f, float y = 0.5f);
    void setViewport(const mnd::MandelViewport& viewport);
    void setMaxIterations(int maxIter);

    //void redraw();

    void requestRecalc(void);

    void resizeEvent(QResizeEvent* re) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void wheelEvent(QWheelEvent * we) override;

    inline const mnd::MandelViewport& getViewport(void) const { return targetViewport; }
signals:
    void needsUpdate(const mnd::MandelInfo vp);
public slots:
    //void viewUpdated(Bitmap<RGBColor>* bitmap);
};

