#ifndef ESCAPETIMEVISUALWIDGET_H
#define ESCAPETIMEVISUALWIDGET_H

#include <QOpenGLWidget>

class QOpenGLShaderProgram;


class EscapeTimeVisualWidget;


class ETVImage
{
    GLuint textureId;
    EscapeTimeVisualWidget& owner;
public:
    ETVImage(EscapeTimeVisualWidget& owner);
    ~ETVImage(void);

    void draw(float x, float y, float w, float h,
              float tx = 0.0f, float ty = 0.0f,
              float tw = 1.0f, float th = 1.0f);
};


class EscapeTimeVisualWidget :
    public QOpenGLWidget
{
    Q_OBJECT

    friend class ETVImage;
    QOpenGLShaderProgram* program;
    GLuint gradientTextureId;
public:
    EscapeTimeVisualWidget(QWidget* parent = nullptr);

    void initializeGL(void) override;
    void resizeGL(int w, int h) override;
    void paintGL(void) override;
};

#endif // ESCAPETIMEVISUALWIDGET_H
