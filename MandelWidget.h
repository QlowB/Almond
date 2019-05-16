#pragma once

#include <QGLWidget>
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
#include <atomic>

class Texture
{
    GLuint id;
public:
    Texture(const Bitmap<RGBColor>& pict);
    ~Texture(void);

    void bind(void) const;

    void drawRect(float x, float y, float width, float height);
};

class MandelView : public QObject
{
    Q_OBJECT
private:
    std::future<void> calc;
    std::atomic<mnd::MandelViewport> toCalc;
    std::atomic_bool hasToCalc;
    mnd::Generator& generator;
public:
    inline MandelView(mnd::Generator& generator) :
        generator{ generator }
    {
    }
public slots:
    void adaptViewport(const mnd::MandelViewport& vp);
signals:
    void updated(const Bitmap<RGBColor>* bitmap);
};

class MandelWidget : public QGLWidget
{
    Q_OBJECT
private:
    //QScrollArea qsa;
    //QLabel ql;
    mnd::MandelContext& mndContext;

    bool initialized = false;

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

    //void redraw();

    void resizeEvent(QResizeEvent* re) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

    inline const mnd::MandelViewport& getViewport(void) const { return viewport; }
signals:
    void needsUpdate(const mnd::MandelViewport& vp);
public slots:
    void viewUpdated(const Bitmap<RGBColor>* bitmap);
};

