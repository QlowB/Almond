#pragma once

#include <QGLWidget>
#include <QThread>
#include <qopengl.h>
#include <qopenglfunctions.h>
#include <qopenglcontext.h>
#include <qscrollarea.h>
#include <qlabel.h>
#include <qevent.h>
#include <qrubberband.h>

#include "Bitmap.h"
#include <Mandel.h>

#include <future>
#include <thread>
#include <mutex>
#include <atomic>

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

    void drawRect(float x, float y, float width, float height);
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
    MandelWidget* mWidget;
    //QOpenGLContext* context;
public:
    MandelView(mnd::Generator& generator, MandelWidget* mWidget);
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

class MandelWidget : public QGLWidget
{
    Q_OBJECT
private:
    //QScrollArea qsa;
    //QLabel ql;
    mnd::MandelContext& mndContext;

    bool initialized = false;
    int maxIterations = 2000;

    bool rubberbandDragging = false;
    QRectF rubberband;

    std::unique_ptr<Texture> tex;
    mnd::MandelViewport viewport;
    MandelView mv;
public:
    MandelWidget(mnd::MandelContext& ctxt, QWidget* parent = nullptr);
    ~MandelWidget(void) override;


    /*inline MandelWidget(const MandelWidget& other) :
        mndContext{ other.mndContext },
        mv{ other.mndContext }
    {
    }*/

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

