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
    ETVImage(EscapeTimeVisualWidget& owner,
             QOpenGLContext* context,
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
protected:
    QOpenGLShaderProgram* program;
    QOpenGLShaderProgram* renderTextures;
    QOpenGLShaderProgram* juliaPreviewer;
    GLuint gradientTextureId;
    float gradientTextureMax;
    float maxIterations;
    Gradient gradient;
    bool gradientNeedsUpdate;

    float resolutionX;
    float resolutionY;

    GLuint tileFramebuffer;
    GLuint tileTexture;

public:
    EscapeTimeVisualWidget(QWidget* parent = nullptr);

    void setGradient(Gradient newGradient);
    const Gradient& getGradient(void);

    virtual void initializeGL(void) override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL(void) override;
    void drawJulia(float jx, float jy, QRectF area, bool drawSmooth);

    void setMaxIterationCutoff(float maxIter);


    void setResolutionX(int w);
    void setResolutionY(int h);
    int getResolutionX(void) const;
    int getResolutionY(void) const;
private:
    void updateGradient(void);
};

#endif // ESCAPETIMEVISUALWIDGET_H
