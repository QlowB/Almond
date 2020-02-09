#pragma once

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QThread>
#include <qopengl.h>
#include <qopenglfunctions.h>
#include <qopenglcontext.h>
#include <qscrollarea.h>
#include <qlabel.h>
#include <qevent.h>
#include <qrubberband.h>

#include "Bitmap.h"
#include "Gradient.h"
#include <Mandel.h>

#include <future>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

class MandelWidget;

class Texture
{
    GLuint id;
    QOpenGLContext* context;
public:
    Texture(const Bitmap<RGBColor>& pict);
    Texture(const Bitmap<RGBColor>& pict, QOpenGLContext* context);
    ~Texture(void);

    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture(Texture&& other) = default;
    Texture& operator=(Texture&& other) = default;

    void bind(void) const;
    inline GLuint getId(void) const { return id; }

    void drawRect(float x, float y, float width, float height);
};


struct MandelClip
{
    mnd::MandelViewport view;
};

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return (h1 ^ 234579245) * 23452354 + h2;
    }
};


class TexGrid
{
    double dpp;
    std::unordered_map<std::pair<int, int>, Texture, PairHash> cells;
public:
    inline TexGrid(void) : dpp{ 1.0 } {}
    inline TexGrid(double dpp) : dpp{ dpp } {}
    std::pair<int, int> getCellIndices(double x, double y);
    std::pair<double, double> getPositions(int i, int j);
    Texture* getCell(int i, int j);
};


class MandelV
{
public:
    Texture empty;
    std::unordered_map<int, TexGrid> levels;
public:
    MandelV(QOpenGLContext* context);
    int getLevel(double dpp);
    double getDpp(int level);

    TexGrid& getGrid(int level);

    void paint(const mnd::MandelViewport& mvp);
};


class MandelView : public QObject
{
    Q_OBJECT
private:
    std::future<void> calc;
    QThread calcThread;
    std::mutex mut;
    std::condition_variable condVar;
    std::atomic<mnd::MandelInfo> toCalc;
    std::atomic_bool hasToCalc;
    std::atomic_bool finish;
    mnd::Generator* generator;
    Gradient& gradient;
    MandelWidget* mWidget;
    //QOpenGLContext* context;
public:
    MandelView(mnd::Generator& generator, Gradient& gradient, MandelWidget* mWidget);
    ~MandelView(void);

    void setGenerator(mnd::Generator &value);

    void start();
private slots:
    void loop();

public slots:
    void adaptViewport(const mnd::MandelInfo vp);
signals:
    void updated(Bitmap<RGBColor>* bitmap);
};

class MandelWidget : public QOpenGLWidget
{
    Q_OBJECT
private:
    //QScrollArea qsa;
    //QLabel ql;
    mnd::MandelContext& mndContext;

    Gradient gradient;

    bool initialized = false;
    int maxIterations = 2000;

    bool rubberbandDragging = false;
    QRectF rubberband;

    std::unique_ptr<Texture> tex;
    mnd::MandelViewport viewport;
    MandelView mv;
    std::unique_ptr<MandelV> v;
public:
    MandelWidget(mnd::MandelContext& ctxt, QWidget* parent = nullptr);
    ~MandelWidget(void) override;


    /*inline MandelWidget(const MandelWidget& other) :
        mndContext{ other.mndContext },
        mv{ other.mndContext }
    {
    }*/

    inline const Gradient& getGradient(void) const { return gradient; }

    void initializeGL(void) override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void drawRubberband(void);

    void zoom(float scale);
    void setMaxIterations(int maxIter);

    //void redraw();

    void requestRecalc(void);

    void resizeEvent(QResizeEvent* re) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

    inline const mnd::MandelViewport& getViewport(void) const { return viewport; }
signals:
    void needsUpdate(const mnd::MandelInfo vp);
public slots:
    void viewUpdated(Bitmap<RGBColor>* bitmap);
};

