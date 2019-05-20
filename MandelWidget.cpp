#include "MandelWidget.h"
#include <cmath>

using namespace mnd;

#include <QOpenGLVertexArrayObject>


Texture::Texture(const Bitmap<RGBColor>& bitmap, QOpenGLContext* context) :
    context{ context }
{
    context->functions()->glGenTextures(1, &id);
    context->functions()->glBindTexture(GL_TEXTURE_2D, id);

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
    context->functions()->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, int(bitmap.width), int(bitmap.height), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    context->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    context->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    context->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    context->functions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


Texture::~Texture(void)
{
    context->functions()->glDeleteTextures(1, &id);
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


MandelView::MandelView(mnd::Generator& generator, MandelWidget* mWidget) :
    generator{ &generator },
    mWidget{ mWidget },
    context{ new QOpenGLContext(this) }
{
    context->setShareContext(mWidget->context()->contextHandle());
}


void MandelView::setGenerator(mnd::Generator& value)
{
    generator = &value;
}

void MandelView::adaptViewport(const MandelInfo mi)
{
    //bmp->get(0, 0) = RGBColor{ 10, uint8_t(sin(1 / vp.width) * 127 + 127), 10 };
    /*printf("adapted\n");
    if (calc.valid()) {
        auto status = calc.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::deferred) {
            printf("deferred\n");
        } else if (status == std::future_status::timeout) {
            printf("timeout\n");
        } else if (status == std::future_status::ready) {
            printf("ready!\n");
        }
    }*/
    if (!calc.valid() || calc.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        toCalc = mi;
        hasToCalc = true;
        calc = std::async([this, mi] () {
            QOpenGLContext* context = new QOpenGLContext();
            context->setShareContext(mWidget->context()->contextHandle());
             while(hasToCalc.exchange(false)) {
                auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
                generator->generate(mi, fmap.pixels.get());
                auto bitmap = fmap.map<RGBColor>([&mi](float i) { return i > mi.maxIter ?
                                RGBColor{ 0,0,0 } :
                                RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127),
                                          uint8_t(sin(i * 0.01f) * 127 + 127),
                                          uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });

                Texture* tex = new Texture(bitmap, context);
                emit updated(tex);
            }
        });
    }
    else {
        toCalc = mi;
        hasToCalc = true;
    }
}


MandelWidget::MandelWidget(mnd::MandelContext& ctxt, QWidget* parent) :
    QGLWidget{ QGLFormat(QGL::SampleBuffers), parent },
    mndContext{ ctxt },
    mv{ ctxt.getDefaultGenerator(), this }
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setSizePolicy(QSizePolicy::Expanding,
        QSizePolicy::Expanding);
    QObject::connect(&mv, &MandelView::updated, this, &MandelWidget::viewUpdated, Qt::AutoConnection);
    QObject::connect(this, &MandelWidget::needsUpdate, &mv, &MandelView::adaptViewport, Qt::AutoConnection);

    if (!ctxt.getDevices().empty()) {
        if (auto* gen = ctxt.getDevices()[0].getGeneratorDouble(); gen) {
            mv.setGenerator(*gen);
        }
    }
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
    bitmap.get(0, 0) = RGBColor{50, 50, 50};

    tex = std::make_unique<Texture>(bitmap, context()->contextHandle());
    emit needsUpdate(MandelInfo{ viewport, this->width(), this->height(), 2000 });
}


void MandelWidget::paintGL(void)
{
    /*if (!initialized) {
        emit needsUpdate(viewport);
        initialized = true;
    }*/

    int width = this->width();
    int height = this->height();

    /*CpuGenerator<double> cpg;
    ClGenerator clg;
    MandelGenerator& mg = cpg;
    MandelInfo mi;
    mi.bWidth = width;
    mi.bHeight = height;
    mi.maxIter = 5000;
    mi.view = viewport;*/
    //auto bitmap = mg.generate(mi);
    /*Bitmap<RGBColor> bitmap(1000, 1000);
    for (int i = 0; i < 1000 * 1000; i++)
        bitmap.pixels[i] = RGBColor{5, uint8_t((i % 1000) ^ (i / 1000)), 50};
    tex = std::make_unique<Texture>(bitmap);*/

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

    if (rubberbandDragging)
        drawRubberband();

    printf("painted GL\n");
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


/*void MandelWidget::redraw(void)
{
    /*CpuGenerator<double> cpg;
    MandelInfo mi;
    mi.bWidth = this->geometry().width();//ql.geometry().width();
    mi.bHeight = this->geometry().height(); //ql.geometry().height();
    mi.maxIter = 250;
    mi.view = viewport;*/
    //update();
    //emit needsUpdate(viewport);

    //auto bitmap = cpg.generate(mi).map<uint32_t>([](RGBColor rgb) { return 255 << 24 | rgb.b << 16 | rgb.g << 8 | rgb.r; });
//}


void MandelWidget::resizeEvent(QResizeEvent* re)
{
    double aspect = double(geometry().width()) / geometry().height();

    //if (viewport.width > viewport.height * aspect)
        viewport.height = (viewport.width / aspect);
    //else
    //    viewport.width = (viewport.height * aspect);

    emit needsUpdate(MandelInfo{ viewport, this->width(), this->height(), 2000 });
    //redraw();
}


void MandelWidget::mousePressEvent(QMouseEvent* me)
{
    rubberband.setCoords(me->x(), me->y(), 0, 0);
    rubberbandDragging = true;
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
    if (rubberbandDragging)
        emit repaint();
}


void MandelWidget::mouseReleaseEvent(QMouseEvent* me)
{
    QRect rect = rubberband.toRect();
    QRect full = this->geometry();

    viewport.x += double(rect.left()) * viewport.width / full.width();
    viewport.y += double(rect.top()) * viewport.height / full.height();
    viewport.width *= double(rect.width()) / full.width();
    viewport.height *= double(rect.height()) / full.height();
    viewport.normalize();
    rubberbandDragging = false;
    emit needsUpdate(MandelInfo{ viewport, this->width(), this->height(), 2000 });
}


void MandelWidget::viewUpdated(Texture* bitmap)
{
    tex = std::unique_ptr<Texture>(bitmap);//std::make_unique<Texture>(*bitmap);
    //delete bitmap;
    printf("viewUpdated\n");
    emit repaint();
}
