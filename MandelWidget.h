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
#include <Mandel.h>

#include "Bitmap.h"
#include "Gradient.h"

#include <atomic>
#include <tuple>
#include <deque>
#include <chrono>
#include <unordered_map>
#include <unordered_set>


using GridIndex = mnd::Integer;
Q_DECLARE_METATYPE(GridIndex)
Q_DECLARE_METATYPE(mnd::Real)


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
    CellImage(const CellImage& b) = delete;
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

    TextureClip(TextureClip&&) = default;
    TextureClip(const TextureClip&) = delete;

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

    QuadImage(QuadImage&&) = default;
    QuadImage(const QuadImage&) = delete;

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
        //boost::hash_combine(h1, p.second);
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
    void clearUncleanCells(void);
};


class Job : public QObject, public QRunnable
{
    Q_OBJECT
public:
    mnd::MandelGenerator* generator;
    const Gradient& gradient;
    MandelWidget& owner;
    TexGrid* grid;
    int level;
    GridIndex i, j;
    long calcState = 0;

    inline Job(mnd::MandelGenerator* generator,
               const Gradient& gradient,
               MandelWidget& owner,
               TexGrid* grid,
               int level, GridIndex i, GridIndex j,
               long calcState) :
        generator{ generator },
        gradient{ gradient },
        owner{ owner },
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
    mnd::MandelGenerator* generator;
    std::unique_ptr<QThreadPool> threadPool;
    MandelWidget& owner;
    const Gradient& gradient;
    int currentLevel;

    volatile unsigned int calcState = 0;
public:
    Calcer(mnd::MandelGenerator* generator, MandelWidget& owner);
    void clearAll(void);
    void setGenerator(mnd::MandelGenerator* generator) { this->generator = generator; changeState(); }

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
    mnd::MandelGenerator* generator;
    Calcer calcer;
    MandelWidget& owner;
    int width;
    int height;
public:
    static const int chunkSize;
    MandelView(mnd::MandelGenerator* generator, MandelWidget& owner);
    int getLevel(mnd::Real dpp);
    mnd::Real getDpp(int level);

    TexGrid& getGrid(int level);

    void setGenerator(mnd::MandelGenerator* generator);

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
    mnd::MandelGenerator* generator;
    mnd::MandelInfo mandelInfo;

    bool initialized = false;

    Gradient gradient;

    volatile bool selectingPoint = false;
    float pointX;
    float pointY;

    volatile bool rubberbanding = false;
    QRectF rubberband;
    volatile bool dragging = false;
    int dragX, dragY;

    bool displayInfo = false;

    mnd::MandelViewport currentViewport;
    mnd::MandelViewport targetViewport;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastAnimUpdate;

    std::unique_ptr<MandelView> mandelView;
public:
    MandelWidget(mnd::MandelContext& ctxt, mnd::MandelGenerator* generator, QWidget* parent = nullptr);
    ~MandelWidget(void) override;

    inline const Gradient& getGradient(void) const { return gradient; }
    void setGradient(Gradient g);

    inline bool getSmoothColoring(void) const { return mandelInfo.smooth; }
    void setSmoothColoring(bool sc);

    inline bool doesDisplayInfo(void) const { return displayInfo; }
    void setDisplayInfo(bool di);

    inline int getMaxIterations(void) const { return mandelInfo.maxIter; }
    void setMaxIterations(int maxIter);

    inline const mnd::MandelInfo& getMandelInfo(void) const { return mandelInfo; }
    inline mnd::MandelInfo& getMandelInfo(void) { return mandelInfo; }

    void setJuliaPos(const mnd::Real& x, const mnd::Real& y);
    const mnd::Real& getJuliaX(void) { return mandelInfo.juliaX; }
    const mnd::Real& getJuliaY(void) { return mandelInfo.juliaY; }

    inline mnd::MandelGenerator* getGenerator(void) const { return generator; }
    void setGenerator(mnd::MandelGenerator* generator);
    void clearAll(void);

    void initializeGL(void) override;
    void paintGL() override;

private:
    void updateAnimations(void);

    void drawRubberband(void);
    void drawInfo(void);
    void drawPoint(void);
public:

    void zoom(float scale, float x = 0.5f, float y = 0.5f);
    void setViewport(const mnd::MandelViewport& viewport);
    void selectPoint(void);

    void requestRecalc(void);

    void resizeEvent(QResizeEvent* re) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void wheelEvent(QWheelEvent * we) override;

    inline const mnd::MandelViewport& getViewport(void) const { return targetViewport; }
signals:
    void needsUpdate(const mnd::MandelInfo vp);
    void pointSelected(mnd::Real x, mnd::Real y);
};

