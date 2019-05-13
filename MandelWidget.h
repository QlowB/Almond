#pragma once

#include <QGLWidget>
#include <qopengl.h>
#include <qopenglfunctions.h>
#include <qopenglcontext.h>
#include <qscrollarea.h>
#include <qlabel.h>
#include <qevent.h>
#include <qrubberband.h>
#include "Generators.h"

class Texture
{
    GLuint id;
public:
    Texture(const Bitmap<RGBColor>& pict);
    ~Texture(void);

    void bind(void) const;

    void drawRect(float x, float y, float width, float height);
};

class MandelWidget : public QGLWidget
{
    Q_OBJECT
private:
    //QScrollArea qsa;
    //QLabel ql;
    QRectF rubberband;

    std::unique_ptr<Texture> tex;
    MandelViewport viewport;
public:
    MandelWidget(QWidget* parent = nullptr);
    ~MandelWidget(void) override;

    void initializeGL(void) override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void drawRubberband(void);

    void redraw();

    void resizeEvent(QResizeEvent* re) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

    inline const MandelViewport& getViewport(void) const { return viewport; }
signals:
};

