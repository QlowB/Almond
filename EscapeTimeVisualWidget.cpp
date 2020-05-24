#include "EscapeTimeVisualWidget.h"

#include "Bitmap.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <vector>


ETVImage::ETVImage(EscapeTimeVisualWidget& owner,
                   const Bitmap<float>& img) :
    owner{ owner }
{
    auto& gl = *owner.context()->functions();
    gl.glGenTextures(1, &textureId);
    gl.glActiveTexture(GL_TEXTURE0);
    gl.glBindTexture(GL_TEXTURE_2D, textureId);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, int(img.width), int(img.height), 0, GL_RED, GL_FLOAT, img.pixels.get());
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.glBindTexture(GL_TEXTURE_2D, 0);
}


ETVImage::~ETVImage(void)
{
    auto& gl = *owner.context()->functions();
    gl.glDeleteTextures(1, &textureId);
}


void ETVImage::draw(float x, float y, float w, float h,
                    float tx, float ty, float tw, float th)
{
    auto& gl = *owner.context()->functions();

    GLfloat const vertices[] = {
        x, y,  0.0f,
        x, y + h, 0.0f,
        x + w, y, 0.0f,
        x + w, y + h, 0.0f,
    };

    GLfloat const texCoords[] = {
        tx,      ty,
        tx,      ty + th,
        tx + tw, ty,
        tx + tw, ty + th,
    };

    QColor color{ 255, 255, 255 };
    auto& program = owner.program;
    int vertexLoc = program->attributeLocation("vertex");
    int texCoordsLoc = program->attributeLocation("texCoord");
    int colorLocation = program->uniformLocation("color");
    int texLoc = program->uniformLocation("tex");
    int gradLoc = program->uniformLocation("gradient");
    program->setAttributeArray(vertexLoc, vertices, 3);
    program->setAttributeArray(texCoordsLoc, texCoords, 2);
    program->enableAttributeArray(vertexLoc);
    program->enableAttributeArray(texCoordsLoc);
    program->setUniformValue(colorLocation, color);


    gl.glEnable(GL_TEXTURE_2D);

    gl.glUniform1i(texLoc, 0);
    gl.glUniform1i(gradLoc, 2);

    gl.glActiveTexture(GL_TEXTURE0);
    gl.glBindTexture(GL_TEXTURE_2D, textureId);
    gl.glActiveTexture(GL_TEXTURE2);
    gl.glBindTexture(GL_TEXTURE_2D, owner.gradientTextureId);

    gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    program->disableAttributeArray(vertexLoc);
    program->disableAttributeArray(texCoordsLoc);
    gl.glActiveTexture(GL_TEXTURE0);
}


EscapeTimeVisualWidget::EscapeTimeVisualWidget(QWidget* parent) :
    QOpenGLWidget{ parent },
    gradientNeedsUpdate{ false }
{
}


void EscapeTimeVisualWidget::setGradient(Gradient newGradient)
{
    this->gradient = newGradient;
    gradientNeedsUpdate = true;
    update();
}


const Gradient& EscapeTimeVisualWidget::getGradient(void)
{
    return gradient;
}


void EscapeTimeVisualWidget::initializeGL(void)
{
    auto& gl = *this->context()->functions();
    gl.glClearColor(0, 0, 0, 0);

    gl.glDisable(GL_DEPTH_TEST);

    // looks not even better
    //gl.glEnable(GL_FRAMEBUFFER_SRGB);

    //glShadeModel(GL_SMOOTH);

    program = new QOpenGLShaderProgram{ this->context() };
    bool vert = program->addShaderFromSourceCode(QOpenGLShader::Vertex,
    "attribute highp vec4 vertex;\n"
    "attribute highp vec2 texCoord;\n"
    "uniform highp mat4 matrix;\n"
    "varying highp vec2 texc;\n"
    "void main(void)\n"
    "{\n"
    "   gl_Position = matrix * vertex;\n"
    "   texc = texCoord;\n"
    "}");

    // TODO rewrite this monster
    bool frag = program->addShaderFromSourceCode(QOpenGLShader::Fragment,
    "#version 400\n"
    "uniform sampler2D gradient;\n"
    "uniform sampler2D tex;\n"
    "uniform mediump vec4 color;\n"
    "uniform highp float gradientScaler;\n"
    "varying highp vec2 texc;\n"
    "void main(void)\n"
    "{\n"
    "   float v = texture2D(tex, texc).r;\n"
    "   vec2 size = textureSize(tex, 0);\n"
    //"   size = vec2(256.0, 256.0);\n"
    "   vec2 accPoint = texc * size;\n"
    "   vec2 ip = floor(accPoint);\n"
    "   vec2 fp = fract(accPoint);\n"
    "   vec4 inter = textureGather(tex, ip / size, 0);\n"
    "   vec4 col1 = texture2D(gradient, vec2(inter.x*0.005, 0.0));\n"
    "   vec4 col2 = texture2D(gradient, vec2(inter.y*0.005, 0.0));\n"
    "   vec4 col3 = texture2D(gradient, vec2(inter.z*0.005, 0.0));\n"
    "   vec4 col4 = texture2D(gradient, vec2(inter.w*0.005, 0.0));\n"
    "   vec4 col = mix(mix(col4, col3, fp.x), mix(col1, col2, fp.x), fp.y);\n"
    "   gl_FragColor = texture2D(gradient, vec2(v*0.005, 0.0));\n"
    "   gl_FragColor = col;\n"
//    "   gl_FragColor = gl_FragColor * texture2D(tex, texc);\n"
//    "   float v = texture2D(tex, texc).r;\n"
//    "   gl_FragColor = vec4(v, 1.0 - v, v*v, 1);\n"
//    "   gl_FragColor.g = 0.3;\n"
    "}");
    
    //program.link();
    bool bound = program->bind();

    int vertexLoc = program->attributeLocation("vertex");
    int texCoordsLoc = program->attributeLocation("texCoord");
    int colorLocation = program->uniformLocation("color");
    int texLoc = program->uniformLocation("tex");
    int gradLoc = program->uniformLocation("gradient");
    int gradientScaler = program->uniformLocation("gradientScaler");


    unsigned char pix[] = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };

    GLuint id;
    gl.glEnable(GL_TEXTURE_2D);
    gl.glGenTextures(1, &id);
    gl.glBindTexture(GL_TEXTURE_2D, id);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 3, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<char*> (pix));
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glBindTexture(GL_TEXTURE_2D, 0);

    gradientTextureId = id;

    //gl3.glBindSampler(0, id);
}


void EscapeTimeVisualWidget::resizeGL(int w, int h)
{
    auto& gl = *this->context()->functions();
    float pixelRatio = this->devicePixelRatioF();
    //pixelRatio = 1.0 / 32;

    float newW = w * pixelRatio;
    float newH = h * pixelRatio;

    setResolutionX(newW);
    setResolutionY(newH);

    gl.glViewport(0, 0, newW, newH);

    QMatrix4x4 pmvMatrix;
    pmvMatrix.ortho(QRectF{ 0, 0, newW, newH });
    int matrixLocation = program->uniformLocation("matrix");
    program->setUniformValue(matrixLocation, pmvMatrix);
}


void EscapeTimeVisualWidget::paintGL(void)
{
    if (gradientNeedsUpdate)
        updateGradient();
    /*ETVImage etvi{ *this };

    auto& gl = *this->context()->functions();
    gl.glClearColor(0.0, 0.2, 0.0, 1.0);
    gl.glClear(GL_COLOR_BUFFER_BIT);

    etvi.draw(100, 100, 700, 700);*/
}


void EscapeTimeVisualWidget::updateGradient(void)
{
    auto& gl = *this->context()->functions();

    const int len = 512;
    std::unique_ptr<uint8_t[]> pixels = std::make_unique<uint8_t[]>(len * 3);

    for (int i = 0; i < len; i++) {
        RGBColor c = gradient.get(gradient.getMax() * i / len);
        pixels[i * 3] = c.r;
        pixels[i * 3 + 1] = c.g;
        pixels[i * 3 + 2] = c.b;
    }

    gl.glEnable(GL_TEXTURE_2D);
    gl.glBindTexture(GL_TEXTURE_2D, gradientTextureId);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, len, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<unsigned char*> (pixels.get()));
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glBindTexture(GL_TEXTURE_2D, 0);

    gradientNeedsUpdate = false;
}


void EscapeTimeVisualWidget::setResolutionX(int w)
{
    resolutionX = w;
}


void EscapeTimeVisualWidget::setResolutionY(int h)
{
    resolutionY = h;
}


int EscapeTimeVisualWidget::getResolutionX(void) const
{
    return resolutionX;
}


int EscapeTimeVisualWidget::getResolutionY(void) const
{
    return resolutionY;
}
