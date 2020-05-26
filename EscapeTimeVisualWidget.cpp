#include "EscapeTimeVisualWidget.h"

#include "Bitmap.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_4_0_Core>

#include <vector>


ETVImage::ETVImage(EscapeTimeVisualWidget& owner,
                   const Bitmap<float>& img) :
    owner{ owner }
{
    auto& gl = *QOpenGLContext::currentContext()->functions();
    gl.glGenTextures(1, &textureId);
    gl.glActiveTexture(GL_TEXTURE0);
    gl.glBindTexture(GL_TEXTURE_2D, textureId);

// workaround to weird bug appearing on OS X that the first time
// a texture is displayed, it seems to display arbitrary graphics data
// (e.g. parts of other windows or even old mandelbrot textures)
//
// bug doesn't appear if glTexImage2D is called twice with the image data
#ifdef __APPLE__
    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, int(img.width), int(img.height), 0, GL_RED, GL_FLOAT, img.pixels.get());
#endif // __APPLE__
    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, int(img.width), int(img.height), 0, GL_RED, GL_FLOAT, img.pixels.get());
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    auto& gle = *owner.context()->extraFunctions();

    GLfloat const fbVertices[] = {
        0, 0,  0.0f,
        0, 256, 0.0f,
        256, 0, 0.0f,
        256, 256, 0.0f,
    };

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

    GLfloat const fullTexCoords[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1,
    };

    QColor color{ 255, 255, 255 };
    auto& program = owner.program;
    program->bind();
    int vertexLoc = program->attributeLocation("vertex");
    int texCoordsLoc = program->attributeLocation("texCoord");
    int colorLocation = program->uniformLocation("color");
    int texLoc = program->uniformLocation("tex");
    int gradLoc = program->uniformLocation("gradient");
    int gradientScaler = program->uniformLocation("gradientScaler");
    int maxIterations = program->uniformLocation("maxIterations");
    program->setAttributeArray(vertexLoc, vertices, 3);
    program->setAttributeArray(texCoordsLoc, texCoords, 2);
    program->enableAttributeArray(vertexLoc);
    program->enableAttributeArray(texCoordsLoc);
    program->setUniformValue(colorLocation, color);
    program->setUniformValue(gradientScaler, 1.0f / float(owner.gradientTextureMax));
    program->setUniformValue(maxIterations, float(owner.maxIterations));


    QMatrix4x4 pmvMatrix;
    pmvMatrix.ortho(QRect{ 0, 0, owner.getResolutionX(), owner.getResolutionY() });
    int matrixLocation = program->uniformLocation("matrix");
    program->setUniformValue(matrixLocation, pmvMatrix);

    gl.glEnable(GL_TEXTURE_2D);

    //GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    //gle.glDrawBuffers(1, drawBuffers);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, owner.tileFramebuffer);
    gl.glDisable(GL_DEPTH_TEST);
    if(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("error intitializing framebuffer\n");
    }
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, owner.tileTexture, 0);
    //gl.glViewport(0, 0, 256, 256);

    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl.glUniform1i(texLoc, 0);
    gl.glUniform1i(gradLoc, 2);

    gl.glActiveTexture(GL_TEXTURE0);
    gl.glBindTexture(GL_TEXTURE_2D, textureId);
    gl.glActiveTexture(GL_TEXTURE2);
    gl.glBindTexture(GL_TEXTURE_2D, owner.gradientTextureId);

    gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    program->disableAttributeArray(vertexLoc);
    program->disableAttributeArray(texCoordsLoc);

    /*owner.renderTextures->bind();
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //gl.glViewport(0, 0, owner.getResolutionX(), owner.getResolutionY());
    int rtVertexLoc = owner.renderTextures->attributeLocation("vertex");
    int rtTexCoordsLoc = owner.renderTextures->attributeLocation("texCoord");
    int rtTexLoc = owner.renderTextures->attributeLocation("tex");

    gl.glActiveTexture(GL_TEXTURE0);
    gl.glUniform1i(rtTexLoc, 0);
    owner.renderTextures->setAttributeArray(rtVertexLoc, vertices, 3);
    owner.renderTextures->setAttributeArray(rtTexCoordsLoc, fullTexCoords, 2);
    owner.renderTextures->enableAttributeArray(rtVertexLoc);
    owner.renderTextures->enableAttributeArray(rtTexCoordsLoc);
    gl.glBindTexture(GL_TEXTURE_2D, owner.tileTexture);
    //if (rand() % 2)
    //gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    owner.renderTextures->disableAttributeArray(rtVertexLoc);
    owner.renderTextures->disableAttributeArray(rtTexCoordsLoc);
    */
    gl.glActiveTexture(GL_TEXTURE0);
}


EscapeTimeVisualWidget::EscapeTimeVisualWidget(QWidget* parent) :
    QOpenGLWidget{ parent },
    gradientTextureId{ 0 },
    gradientTextureMax{ 1.0f },
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

    fprintf(stdout, "version: %s\n", gl.glGetString(GL_VERSION));
    fflush(stdout);

    // looks not even better
    //gl.glEnable(GL_FRAMEBUFFER_SRGB);

    //glShadeModel(GL_SMOOTH);

    renderTextures = new QOpenGLShaderProgram{ this->context() };
    renderTextures->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "attribute highp vec2 texCoord;\n"
        "uniform highp mat4 matrix;\n"
        "varying highp vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = matrix * vertex;\n"
        "   texc = texCoord;\n"
        "}");
    renderTextures->addShaderFromSourceCode(QOpenGLShader::Fragment,
//        "#version 110\n"
        "uniform sampler2D tex;\n"
        "varying highp vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(tex, texc);\n"
        "}");

    renderTextures->link();

    juliaPreviewer = new QOpenGLShaderProgram{ this->context() };
    juliaPreviewer->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "attribute highp vec2 texCoord;\n"
        "uniform highp mat4 matrix;\n"
        "varying highp vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = matrix * vertex;\n"
        "   texc = texCoord;\n"
        "}");
    juliaPreviewer->addShaderFromSourceCode(QOpenGLShader::Fragment,
//    "#version 110\n"
    "uniform sampler2D gradient;\n"
    "uniform highp float gradientScaler;\n"
    "const highp float maxIterations = 350.0;\n"
    "varying highp vec2 texc;\n"
    "uniform highp float juliaX;\n"
    "uniform highp float juliaY;\n"
    "const highp float left = -1.5;\n"
    "const highp float right = 1.5;\n"
    "const highp float top = -1.5;\n"
    "const highp float bottom = 1.5;\n"
    "highp float map(highp float a, highp float b, highp float v) {\n"
    "    return (1.0 - v) * a + b * v;\n"
    "}\n"
    "highp float iterate(highp float x, highp float y, highp float ca, highp float cb) {\n"
    "    int k = 0;\n"
    "    highp float a = x;\n"
    "    highp float b = y;\n"
    "    while(k <= int(maxIterations)) {\n"
    "        highp float aa = a * a;\n"
    "        highp float bb = b * b;\n"
    "        highp float abab = 2.0 * a * b;\n"
    "        a = aa - bb + ca;\n"
    "        b = abab + cb;\n"
    "        if (aa + bb >= 16.0) break;\n"
    "        k = k + 1;\n"
    "    }\n"
    "    return float(k) + 1.0 - log2(log(a * a + b * b) * 0.5);\n"
    "}\n"
    "void main(void)\n"
    "{\n"
    "    highp float x = map(left, right, texc.x);\n"
    "    highp float y = map(top, bottom, texc.y);\n"
    "    highp float v = iterate(x, y, juliaX, juliaY);\n"
    "    if (v >= maxIterations) {\n"
    "        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    } else {\n"
    "        highp float vnorm = v * gradientScaler;\n"
    "        gl_FragColor = texture2D(gradient, vec2(vnorm, 0.0));\n"
    "    }\n"
    //"    gl_FragColor = vec4(vnorm, 0.0, 0.0, 0.0);\n"
    "}");
    juliaPreviewer->link();

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
    if (!context()->isOpenGLES() &&
        context()->versionFunctions<QOpenGLFunctions_4_0_Core>() != nullptr) {
        bool frag = program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 400\n"
        "uniform sampler2D gradient;\n"
        "uniform sampler2D tex;\n"
        "uniform mediump vec4 color;\n"
        "uniform highp float gradientScaler;\n"
        "uniform highp float maxIterations;\n"
        "varying highp vec2 texc;\n"
        "vec4 colorize(float pos) {\n"
        "    if (pos >= maxIterations) {\n"
        "        return vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    } else {\n"
        "        return texture2D(gradient, vec2(pos * gradientScaler, 0.0));\n"
        "    }\n"
        "}\n"
        "void main(void)\n"
        "{\n"
        "   vec2 size = textureSize(tex, 0);\n"
        "   size = vec2(256.0, 256.0);\n"
        "   vec2 accPoint = texc * size;\n"
        "   vec2 ip = floor(accPoint);\n"
        "   vec2 fp = fract(accPoint);\n"
        "   vec4 inter = textureGather(tex, ip / size, 0);\n"
        "   vec4 col1 = colorize(inter.x);\n"
        "   vec4 col2 = colorize(inter.y);\n"
        "   vec4 col3 = colorize(inter.z);\n"
        "   vec4 col4 = colorize(inter.w);\n"
        "   vec4 col = mix(mix(col4, col3, fp.x), mix(col1, col2, fp.x), fp.y);\n"
        "   gl_FragColor = col;\n"
    //    "   gl_FragColor = gl_FragColor * texture2D(tex, texc);\n"
    //    "   float v = texture2D(tex, texc).r;\n"
    //    "   gl_FragColor = vec4(v, 1.0 - v, v*v, 1);\n"
    //    "   gl_FragColor.g = 0.3;\n"
        "}");
    }
    else {
        bool frag = program->addShaderFromSourceCode(QOpenGLShader::Fragment,
//        "#version 110\n"
        "uniform sampler2D gradient;\n"
        "uniform sampler2D tex;\n"
        "uniform mediump vec4 color;\n"
        "uniform highp float gradientScaler;\n"
        "uniform highp float maxIterations;\n"
        "varying highp vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "   highp float v = texture2D(tex, texc).r;\n"
        "   if (v >= maxIterations) {\n"
        "       gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "   } else {\n"
        "       gl_FragColor = texture2D(gradient, vec2(v * gradientScaler, 0.0));\n"
        "   }\n"
    //    "   gl_FragColor = gl_FragColor * texture2D(tex, texc);\n"
    //    "   float v = texture2D(tex, texc).r;\n"
    //    "   gl_FragColor = vec4(v, 1.0 - v, v*v, 1);\n"
    //    "   gl_FragColor.g = 0.3;\n"
        "}");
    }

    //program.link();
    bool bound = program->bind();
    bound = renderTextures->bind();

    int vertexLoc = program->attributeLocation("vertex");
    int texCoordsLoc = program->attributeLocation("texCoord");
    int colorLocation = program->uniformLocation("color");
    int texLoc = program->uniformLocation("tex");
    int gradLoc = program->uniformLocation("gradient");
    int gradientScaler = program->uniformLocation("gradientScaler");
    int maxIterations = program->uniformLocation("maxIterations");
    program->setUniformValue(gradientScaler, 0.005f);
    program->setUniformValue(maxIterations, 250.0f);

    auto& gle = *this->context()->extraFunctions();

    gl.glGenTextures(1, &tileTexture);
    gl.glBindTexture(GL_TEXTURE_2D, tileTexture);
    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.glBindTexture(GL_TEXTURE_2D, 0);

    gl.glGenFramebuffers(1, &tileFramebuffer);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, tileFramebuffer);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tileTexture, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    gle.glDrawBuffers(1, drawBuffers);
    if(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("error intitializing framebuffer\n");
    }
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    gl.glDisable(GL_DEPTH_TEST);
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
    int rtMatrixLocation = renderTextures->uniformLocation("matrix");
    int jMatrixLocation = juliaPreviewer->uniformLocation("matrix");
    program->setUniformValue(matrixLocation, pmvMatrix);
    renderTextures->setUniformValue(rtMatrixLocation, pmvMatrix);
    juliaPreviewer->setUniformValue(jMatrixLocation, pmvMatrix);
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


void EscapeTimeVisualWidget::drawJulia(float jx, float jy, QRectF area)
{
    juliaPreviewer->bind();
    int gradLoc = juliaPreviewer->uniformLocation("gradient");
    int gradientScaler = juliaPreviewer->uniformLocation("gradientScaler");
    int juliaX = juliaPreviewer->uniformLocation("juliaX");
    int juliaY = juliaPreviewer->uniformLocation("juliaY");
    int vertexLoc = juliaPreviewer->attributeLocation("vertex");
    int texCoordsLoc = juliaPreviewer->attributeLocation("texCoord");
    int maxIterLoc = juliaPreviewer->attributeLocation("maxIterations");

    const float x = area.x();
    const float y = area.y();
    const float w = area.width();
    const float h = area.height();
    GLfloat const vertices[] = {
        x, y,  0.0f,
        x, y + h, 0.0f,
        x + w, y, 0.0f,
        x + w, y + h, 0.0f,
    };

    GLfloat const texCoords[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1,
    };

    auto& gl = *this->context()->functions();
    gl.glEnable(GL_TEXTURE_2D);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    juliaPreviewer->setAttributeArray(vertexLoc, vertices, 3);
    juliaPreviewer->setAttributeArray(texCoordsLoc, texCoords, 2);
    juliaPreviewer->enableAttributeArray(vertexLoc);
    juliaPreviewer->enableAttributeArray(texCoordsLoc);
    juliaPreviewer->setUniformValue(gradientScaler, 1.0f / float(gradientTextureMax));
    juliaPreviewer->setUniformValue(maxIterLoc, float(250));
    juliaPreviewer->setUniformValue(juliaX, float(jx));
    juliaPreviewer->setUniformValue(juliaY, float(jy));

    gl.glUniform1i(gradLoc, 0);

    gl.glActiveTexture(GL_TEXTURE0);
    gl.glBindTexture(GL_TEXTURE_2D, gradientTextureId);

    gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    juliaPreviewer->disableAttributeArray(vertexLoc);
    juliaPreviewer->disableAttributeArray(texCoordsLoc);
    //juliaPreviewer->release();
    //program->bind();
}


void EscapeTimeVisualWidget::setMaxIterationCutoff(float maxIter)
{
    this->maxIterations = maxIter;
}


void EscapeTimeVisualWidget::updateGradient(void)
{
    auto& gl = *this->context()->functions();

    int len = 512;
    if (gradient.getPoints().size() > 25) {
        len = 2048;
    }
    else if (gradient.getPoints().size() > 7) {
        len = 1024;
    }
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
    this->gradientTextureMax = gradient.getMax();

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
