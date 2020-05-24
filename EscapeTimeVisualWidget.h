#ifndef ESCAPETIMEVISUALWIDGET_H
#define ESCAPETIMEVISUALWIDGET_H

#include <QOpenGLWidget>
#include "Bitmap.h"
#include "Gradient.h"

class QOpenGLShaderProgram;

class EscapeTimeVisualWidget;


class ETVImage
{
    GLuint textureId;
    EscapeTimeVisualWidget& owner;
public:
    ETVImage(EscapeTimeVisualWidget& owner,
             const Bitmap<float>& img);
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
    Gradient gradient;
    bool gradientNeedsUpdate;

    float resolutionX;
    float resolutionY;

public:
    EscapeTimeVisualWidget(QWidget* parent = nullptr);

    void setGradient(Gradient newGradient);
    const Gradient& getGradient(void);

    virtual void initializeGL(void) override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL(void) override;

    void setResolutionX(int w);
    void setResolutionY(int h);
    int getResolutionX(void) const;
    int getResolutionY(void) const;
private:
    void updateGradient(void);
};

#endif // ESCAPETIMEVISUALWIDGET_H
