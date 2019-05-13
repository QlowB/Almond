#include "MandelWidget.h"

#include <QOpenGLVertexArrayObject>

Texture::Texture(const Bitmap<RGBColor>& bitmap)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    long lineLength = (bitmap.width * 3 + 3) & ~3;

    unsigned char* pixels = new unsigned char[lineLength * bitmap.height];
    for (int i = 0; i < bitmap.width; i++) {
        for (int j = 0; j < bitmap.height; j++) {
            int index = i * 3 + j * lineLength;
            RGBColor c = bitmap.get(i, j);
            pixels[index] = c.r;
            pixels[index + 1] = c.g;
            pixels[index + 2] = c.b;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, int(bitmap.width), int(bitmap.height), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


Texture::~Texture(void)
{
    glDeleteTextures(1, &id);
}


void Texture::bind(void) const
{
    glBindTexture(GL_TEXTURE_2D, id);
}


void Texture::drawRect(float x, float y, float width, float height)
{
    glColor3ub(255, 255, 255);
    glEnable(GL_TEXTURE_2D);
    bind();
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex2f(x, y);
    glTexCoord2f(1, 0);
    glVertex2f(x + width, y);
    glTexCoord2f(0, 1);
    glVertex2f(x, y + height);
    glTexCoord2f(1, 1);
    glVertex2f(x + width, y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


MandelWidget::MandelWidget(QWidget* parent) :
    QGLWidget{ QGLFormat(QGL::SampleBuffers), parent }
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setSizePolicy(QSizePolicy::Expanding,
        QSizePolicy::Expanding);
}


MandelWidget::~MandelWidget()
{
}


void MandelWidget::initializeGL(void)
{
    qglClearColor(Qt::black);

    glDisable(GL_DEPTH_TEST);
    //glShadeModel(GL_SMOOTH);

    /*CpuGenerator<double> cpg;
    MandelInfo mi;
    mi.bWidth = this->width();//ql.geometry().width();
    mi.bHeight = this->height(); //ql.geometry().height();
    mi.maxIter = 250;
    mi.view = viewport;
    auto bitmap = cpg.generate(mi);*/
    Bitmap<RGBColor> bitmap(1, 1);
    bitmap.get(0, 0) = RGBColor{5, 150, 50};

    tex = std::make_unique<Texture>(bitmap);
}


void MandelWidget::paintGL(void)
{
    int width = this->width();
    int height = this->height();

    CpuGenerator<double> cpg;
    ClGenerator clg;
    MandelGenerator& mg = cpg;
    MandelInfo mi;
    mi.bWidth = width; //ql.geometry().width();
    mi.bHeight = height; //ql.geometry().height();
    mi.maxIter = 5000;
    mi.view = viewport;
    auto bitmap = mg.generate(mi);
    /*Bitmap<RGBColor> bitmap(1000, 1000);
    for (int i = 0; i < 1000 * 1000; i++)
        bitmap.pixels[i] = RGBColor{5, uint8_t((i % 1000) ^ (i / 1000)), 50};*/
    tex = std::make_unique<Texture>(bitmap);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef QT_OPENGL_ES_1
    glOrthof(0, width, height, 0, -1.0, 1.0);
#else
    glOrtho(0, width, height, 0, -1.0, 1.0);
#endif
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    tex->drawRect(0, 0, width, height);

    drawRubberband();
    printf("omg\n");
}


void MandelWidget::drawRubberband(void)
{
    glColor3ub(10, 200, 10);
    glBegin(GL_LINE_LOOP);
    glVertex2d(rubberband.x(), rubberband.y());
    glVertex2d(rubberband.right(), rubberband.y());
    glVertex2d(rubberband.right(), rubberband.bottom());
    glVertex2d(rubberband.x(), rubberband.bottom());
    glEnd();
}


void MandelWidget::resizeGL(int width, int height)
{
}


void MandelWidget::redraw(void)
{
    /*CpuGenerator<double> cpg;
    MandelInfo mi;
    mi.bWidth = this->geometry().width();//ql.geometry().width();
    mi.bHeight = this->geometry().height(); //ql.geometry().height();
    mi.maxIter = 250;
    mi.view = viewport;*/
    update();

    //auto bitmap = cpg.generate(mi).map<uint32_t>([](RGBColor rgb) { return 255 << 24 | rgb.b << 16 | rgb.g << 8 | rgb.r; });
}


void MandelWidget::resizeEvent(QResizeEvent* re)
{
    double aspect = double(geometry().width()) / geometry().height();

    //if (viewport.width > viewport.height * aspect)
        viewport.height = (viewport.width / aspect);
    //else
    //    viewport.width = (viewport.height * aspect);
    
    redraw();
}


void MandelWidget::mousePressEvent(QMouseEvent* me)
{
    rubberband.setCoords(me->x(), me->y(), 0, 0);
}


void MandelWidget::mouseMoveEvent(QMouseEvent* me)
{
    QRectF& rect = rubberband;
    float aspect = float(geometry().width()) / geometry().height();
    rect.setBottomRight(QPoint(me->x(), me->y()));
    if (rect.width() > rect.height() * aspect)
        rect.setHeight(rect.width() / aspect);
    else
        rect.setWidth(rect.height() * aspect);
}


void MandelWidget::mouseReleaseEvent(QMouseEvent* me)
{
    QRect rect = rubberband.toRect();
    QRect full = this->geometry();

    viewport.x += double(rect.left()) * viewport.width / full.width();
    viewport.y += double(rect.top()) * viewport.height / full.height();
    viewport.width *= double(rect.width()) / full.width();
    viewport.height *= double(rect.height()) / full.height();
    redraw();
}
