#include "MandelWidget.h"
#include <cmath>

using namespace mnd;

#include <QOpenGLVertexArrayObject>


Texture::Texture(const Bitmap<RGBColor>& bitmap, GLint param) :
    context{ nullptr }
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
}

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
    if (id != 0)
        glDeleteTextures(1, &id);
}


Texture::Texture(Texture&& other) :
    id{ other.id },
    context{ other.context }
{
    other.id = 0;
}


Texture& Texture::operator=(Texture&& other)
{
    this->id = other.id;
    this->context = other.context;
    other.id = 0;
    return *this;
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


std::pair<int, int> TexGrid::getCellIndices(double x, double y)
{
    return { ::floor(x / dpp / MandelV::chunkSize), ::floor(y / dpp / MandelV::chunkSize) };
}


std::pair<double, double> TexGrid::getPositions(int x, int y)
{
    return { x * dpp * MandelV::chunkSize, y * dpp * MandelV::chunkSize };
}


Texture* TexGrid::getCell(int i, int j)
{
    auto cIt = cells.find({i, j});
    if (cIt != cells.end()) {
        return cIt->second.get();
    }
    else {
        return nullptr;
    }
}


void TexGrid::setCell(int i, int j, std::unique_ptr<Texture> tex)
{
    cells[{i, j}] = std::move(tex);
}


void TexGrid::clearCells(void)
{
    cells.clear();
}


void Job::run(void)
{
    auto [absX, absY] = grid->getPositions(i, j);
    double gw = grid->dpp * MandelV::chunkSize;

    Bitmap<float> f(MandelV::chunkSize, MandelV::chunkSize);
    mnd::MandelInfo mi;
    mi.view.x = absX;
    mi.view.y = absY;
    mi.view.width = mi.view.height = gw;
    mi.bWidth = mi.bHeight = MandelV::chunkSize;
    mi.maxIter = maxIter;
    mndContext.getDefaultGenerator().generate(mi, f.pixels.get());
    auto* rgb = new Bitmap<RGBColor>(f.map<RGBColor>([&mi, this](float i) {
        return i >= mi.maxIter ? RGBColor{ 0, 0, 0 } : gradient.get(i);
    }));
    emit done(level, i, j, rgb);
}


void Calcer::setMaxIter(int maxIter)
{
    this->maxIter = maxIter;
    clearAll();
}


void Calcer::clearAll(void)
{
    this->threadPool->clear();
}


void Calcer::calc(TexGrid& grid, int level, int i, int j)
{
    if (jobs.find({ level, i, j }) == jobs.end()) {
        Job* job = new Job(mndContext, gradient, maxIter, &grid, level, i, j);
        connect(job, &Job::done, this, &Calcer::redirect);
        connect(job, &QObject::destroyed, this, [this, level, i, j] () { this->notFinished(level, i, j); });
        jobs.emplace(level, i, j);
        threadPool->start(job);
    }
}


void Calcer::notFinished(int level, int i, int j)
{
    jobs.erase({ level, i, j });
}


void Calcer::redirect(int level, int i, int j, Bitmap<RGBColor>* bmp)
{
    jobs.erase({ level, i, j });
    emit done(level, i, j, bmp);
}


MandelV::MandelV(mnd::MandelContext& mndContext, Gradient& gradient, int maxIter) :
    mndContext{ mndContext },
    calcer{ mndContext, gradient, maxIter },
    gradient{ gradient },
    maxIter{ maxIter }
{
    Bitmap<RGBColor> emp(8, 8);
    for(auto i = 0; i < emp.width; i++) {
        for(auto j = 0; j < emp.height; j++) {
            if((i + j) & 0x1) { // if i+j is odd
                emp.get(i, j) = RGBColor{ 255, 255, 255 };
            }
            else {
                emp.get(i, j) = RGBColor{ 120, 120, 120 };
            }
        }
    }
    empty = std::make_unique<Texture>(emp, GL_NEAREST);
    connect(&calcer, &Calcer::done, this, &MandelV::cellReady);
}


int MandelV::getLevel(double dpp) {
    return int(::log2(dpp / chunkSize));
}


double MandelV::getDpp(int level)
{
    return ::pow(2, level) * chunkSize;
}


TexGrid& MandelV::getGrid(int level)
{
    auto it = levels.find(level);
    if (it != levels.end()) {
        return it->second;
    }
    else {
        levels.insert(std::pair<int, TexGrid>{ level, TexGrid{ getDpp(level) } });
        return levels[level];
    }
}


void MandelV::setMaxIter(int maxIter)
{
    this->maxIter = maxIter;
    calcer.setMaxIter(maxIter);
    clear();
    emit redrawRequested();
}



void MandelV::clear(void)
{
    for(auto& [level, grid] : this->levels) {
        grid.clearCells();
    }
}


void MandelV::garbageCollect(int level)
{
    for(auto& [l, grid] : levels) {
        int dist = ::abs(l - level);
        if (dist > 20) {
            grid.clearCells();
        }
        else if (dist > 10) {
            if (grid.countAllocatedCells() > 50)
                grid.clearCells();
        }
        else if (dist > 3) {
            if (grid.countAllocatedCells() > 150)
                grid.clearCells();
        }
        else if (dist > 0) {
            if (grid.countAllocatedCells() > 350)
                grid.clearCells();
        }
        else {
            if (grid.countAllocatedCells() > 2500)
                grid.clearCells();
        }
    }
}


void MandelV::paint(const mnd::MandelViewport& mvp)
{
    double dpp = mvp.width / width;
    int level = getLevel(dpp) - 1;
    garbageCollect(level);

    auto& grid = getGrid(level);
    double gw = getDpp(level) * chunkSize;

    double w = width * gw / mvp.width;
    //double h = height * gw / mvp.height;
    printf("level: %d, dpp: %f, width: %f\n", level, dpp, w);

    auto [left, top] = grid.getCellIndices(mvp.x, mvp.y);
    auto [right, bottom] = grid.getCellIndices(mvp.right(), mvp.bottom());
    auto [realXLeft, realYTop] = grid.getPositions(left, top);
    realXLeft = (realXLeft - mvp.x) * width / mvp.width;
    realYTop = (realYTop - mvp.y) * height / mvp.height;
    for(int i = left; i <= right; i++) {
        for(int j = top; j <= bottom; j++) {
            double x = realXLeft + (i - left) * w;
            double y = realYTop + (j - top) * w;

            Texture* t = grid.getCell(i, j);
            if (t != nullptr) {
                t->drawRect(x, y, w, w);
                /*glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + w, y);
                glVertex2f(x + w, y + w);
                glVertex2f(x, y + w);
                glEnd();*/
            }
            else {
                calcer.calc(grid, level, i, j);
                this->empty->drawRect(x, y, w, w);
            }
        }
    }
}

void MandelV::cellReady(int level, int i, int j, Bitmap<RGBColor>* bmp)
{
    this->getGrid(level).setCell(i, j, std::make_unique<Texture>(*bmp));
    delete bmp;
    emit redrawRequested();
}


MandelView::MandelView(mnd::Generator& generator, Gradient &gradient, MandelWidget* mWidget) :
    generator{ &generator },
    gradient{ gradient },
    mWidget{ mWidget }
    //context{ new QOpenGLContext(this) }
{
    //context->setShareContext(mWidget->context()->contextHandle());
    hasToCalc.store(false);
    finish.store(false);
}


MandelView::~MandelView(void)
{
    finish.store(true);
    condVar.notify_one();
    //calcThread.wait(100);
    calcThread.wait(100);
    calcThread.terminate();
}


void MandelView::setGenerator(mnd::Generator& value)
{
    generator = &value;
}

void MandelView::start(void)
{
    this->moveToThread(&calcThread);
    connect(&calcThread, SIGNAL(started()), this, SLOT(loop()));
    calcThread.start();
}


void MandelView::loop(void)
{
    printf("thread!\n"); fflush(stdout);
    //QGLWidget* hiddenWidget = new QGLWidget(nullptr, mWidget);
    //hiddenWidget->setVisible(false);
    //hiddenWidget->context()->contextHandle()->moveToThread(&calcThread);
    //QOpenGLContext* context = hiddenWidget->context()->contextHandle();
    //context->setShareContext(mWidget->context()->contextHandle());
    //context->create();
    //printf("sharing: %d\n", QOpenGLContext::areSharing(hiddenWidget->context()->contextHandle(), mWidget->context()->contextHandle()));
    //fflush(stdout);
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::unique_lock<std::mutex> lock(mut);
    while(true) {
        printf("calcing!\n"); fflush(stdout);
        if (finish.load()) {
            break;
        }
        if (hasToCalc.exchange(false)) {
            const MandelInfo& mi = toCalc.load();
            auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
            generator->generate(mi, fmap.pixels.get());
            auto* bitmap = new Bitmap<RGBColor>(fmap.map<RGBColor>([&mi, this](float i) {
                return i >= mi.maxIter ? RGBColor{ 0, 0, 0 } : gradient.get(i);
            }));
                /*return i >= mi.maxIter ?
                            RGBColor{ 0,0,0 } :
                            RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127),
                                      uint8_t(sin(i * 0.01f) * 127 + 127),
                                      uint8_t(i) }; }));//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
*/
            //hiddenWidget->makeCurrent();
            //Texture* tex = new Texture(bitmap);
            //hiddenWidget->doneCurrent();
            //Texture* tex = 0;
            emit updated(bitmap);
        }
        printf("finished calcing!\n"); fflush(stdout);
        condVar.wait(lock);
        printf("waking!\n"); fflush(stdout);
    }
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
    /*if (!calc.valid() || calc.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        toCalc = mi;
        hasToCalc = true;
        calc = std::async([this, mi] () {
            QGLWidget* hiddenWidget = new QGLWidget(nullptr, (QGLWidget*) mWidget);
            QOpenGLContext* context = hiddenWidget->context()->contextHandle();
            hiddenWidget->makeCurrent();
            //context->setShareContext(mWidget->context()->contextHandle());
            //context->create();
            printf("sharing: %d\n", QOpenGLContext::areSharing(context, mWidget->context()->contextHandle()));
            fflush(stdout);
            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            do {
                auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
                generator->generate(mi, fmap.pixels.get());
                auto bitmap = fmap.map<RGBColor>([&mi](float i) { return i > mi.maxIter ?
                                RGBColor{ 0,0,0 } :
                                RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127),
                                          uint8_t(sin(i * 0.01f) * 127 + 127),
                                          uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });

                Texture* tex = new Texture(bitmap, context);
                //Texture* tex = 0;
                emit updated(tex);
            } while(hasToCalc.exchange(false));
        });
    }
    else {*/

    //std::unique_lock<std::mutex> lock(mut, std::try_to_lock);
    toCalc = mi;
    hasToCalc.exchange(true);
    condVar.notify_one();
    //}
}


MandelWidget::MandelWidget(mnd::MandelContext& ctxt, QWidget* parent) :
    QOpenGLWidget{ parent },
    mndContext{ ctxt },
    mv{ ctxt.getDefaultGenerator(), gradient, this }
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setSizePolicy(QSizePolicy::Expanding,
        QSizePolicy::Expanding);
    QObject::connect(&mv, &MandelView::updated, this, &MandelWidget::viewUpdated, Qt::AutoConnection);
    QObject::connect(this, &MandelWidget::needsUpdate, &mv, &MandelView::adaptViewport, Qt::DirectConnection);

    /*if (!ctxt.getDevices().empty()) {
        if (auto* gen = ctxt.getDevices()[0].getGeneratorDouble(); gen) {
            mv.setGenerator(*gen);
        }
    }*/
}


MandelWidget::~MandelWidget()
{
}


void MandelWidget::initializeGL(void)
{
    this->context()->functions()->glClearColor(0, 0, 0, 0);
    this->context()->makeCurrent(nullptr);

    glDisable(GL_DEPTH_TEST);

    // looks not even better
    //glDisable(GL_FRAMEBUFFER_SRGB);

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
    v = nullptr;

    tex = std::make_unique<Texture>(bitmap, context());
    mv.start();
    requestRecalc();


}


void MandelWidget::paintGL(void)
{
    if (v == nullptr) {
        v = std::make_unique<MandelV>(mndContext, gradient, maxIterations);
        QObject::connect(v.get(), &MandelV::redrawRequested, this, static_cast<void(QOpenGLWidget::*)(void)>(&QOpenGLWidget::update));
    }
    /*if (!initialized) {
        emit needsUpdate(viewport);
        initialized = true;
    }*/

    int width = this->width();
    int height = this->height();
    v->width = width;
    v->height = height;

    //v = std::make_unique<MandelV>(context());
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
    //tex->drawRect(0, 0, width, height);

    //v->empty = std::move(*tex)
    //v->empty.bind();
    v->paint(this->viewport);
    //*tex = std::move(v->empty);

    //if (dragging)
    //    drawRubberband();
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


void MandelWidget::zoom(float scale, float x, float y)
{
    viewport.zoom(scale, x, y);
    //viewport.zoomCenter(scale);
    requestRecalc();
}


void MandelWidget::setMaxIterations(int maxIter)
{
    this->maxIterations = maxIter;
    if (v)
        v->setMaxIter(maxIter);
    requestRecalc();
}


void MandelWidget::requestRecalc()
{
    //emit needsUpdate(MandelInfo{ viewport, this->width(), this->height(), maxIterations });
    this->update();
}


void MandelWidget::resizeGL(int width, int height)
{
    //glViewport(0, 0, (GLint) width, (GLint) height);
    this->update();
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
    QOpenGLWidget::resizeEvent(re);
    double aspect = double(geometry().width()) / geometry().height();

    //if (viewport.width > viewport.height * aspect)
        viewport.height = (viewport.width / aspect);
    //else
    //    viewport.width = (viewport.height * aspect);

    if (v.get() != nullptr) {
        v->width = this->width();
        v->height = this->height();
    }
    printf("resized\n");
    requestRecalc();
    //redraw();
}


void MandelWidget::mousePressEvent(QMouseEvent* me)
{
    QOpenGLWidget::mousePressEvent(me);
    rubberband.setCoords(me->x(), me->y(), 0, 0);
    dragging = true;
    dragX = me->x();
    dragY = me->y();
}


void MandelWidget::mouseMoveEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseMoveEvent(me);
    /*QRectF& rect = rubberband;
    float aspect = float(geometry().width()) / geometry().height();
    rect.setBottomRight(QPoint(me->x(), me->y()));
    if (rect.width() > rect.height() * aspect)
        rect.setHeight(rect.width() / aspect);
    else
        rect.setWidth(rect.height() * aspect);*/
    if (dragging) {
        float deltaX = me->x() - dragX;
        float deltaY = me->y() - dragY;

        this->viewport.x -= deltaX * viewport.width / this->width();
        this->viewport.y -= deltaY * viewport.height / this->height();
        dragX = me->x(); dragY = me->y();

        emit repaint();
    }
}


void MandelWidget::mouseReleaseEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseReleaseEvent(me);
    /*QRect rect = rubberband.toRect();
    QRect full = this->geometry();

    viewport.x += double(rect.left()) * viewport.width / full.width();
    viewport.y += double(rect.top()) * viewport.height / full.height();
    viewport.width *= double(rect.width()) / full.width();
    viewport.height *= double(rect.height()) / full.height();
    viewport.normalize();*/
    dragging = false;
    //requestRecalc();
}

void MandelWidget::wheelEvent(QWheelEvent* we)
{
    QOpenGLWidget::wheelEvent(we);
    float x = float(we->x()) / this->width();
    float y = float(we->y()) / this->height();
    float scale = ::pow(0.9975, we->angleDelta().y());
    printf("scale: %f\n", double(scale));
    zoom(scale, x, y);
    we->accept();
}


void MandelWidget::viewUpdated(Bitmap<RGBColor>* bitmap)
{
    if (bitmap != nullptr) {
        tex = std::make_unique<Texture>(*bitmap);
        delete bitmap;
        emit repaint();
    }
}
