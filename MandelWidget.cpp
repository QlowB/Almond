#include "MandelWidget.h"
#include <cmath>
#include <sstream>

using namespace mnd;

#include <QPainter>

#include <cstdio>


Texture::Texture(const Bitmap<RGBColor>& bitmap, GLint param)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    //int lineLength = (bitmap.width * 3 + 3) & ~3;

    /*std::unique_ptr<unsigned char[]> pixels = std::make_unique<unsigned char[]>(lineLength * bitmap.height);
    for (int i = 0; i < bitmap.width; i++) {
        for (int j = 0; j < bitmap.height; j++) {
            int index = i * 3 + j * lineLength;
            RGBColor c = bitmap.get(i, j);
            pixels[index] = c.r;
            pixels[index + 1] = c.g;
            pixels[index + 2] = c.b;
        }
    }*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, int(bitmap.width), int(bitmap.height), 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<char*> (bitmap.pixels.get()));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
}


Texture::~Texture(void)
{
    if (id != 0)
        glDeleteTextures(1, &id);
}


Texture::Texture(Texture&& other) :
    id{ other.id }
{
    other.id = 0;
}


Texture& Texture::operator=(Texture&& other)
{
    this->id = other.id;
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


CellImage::~CellImage(void)
{
}



TextureClip::~TextureClip(void)
{
}

void TextureClip::drawRect(float x, float y, float width, float height)
{
    glColor3ub(255, 255, 255);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->getId());
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(tx, ty);
    glVertex2f(x, y);
    glTexCoord2f(tx + tw, ty);
    glVertex2f(x + width, y);
    glTexCoord2f(tx, ty + th);
    glVertex2f(x, y + height);
    glTexCoord2f(tx + tw, ty + th);
    glVertex2f(x + width, y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


TextureClip TextureClip::clip(float x, float y, float w, float h)
{
    float tx = this->tx + x * this->tw;
    float ty = this->ty + y * this->th;
    float tw = this->tw * w;
    float th = this->th * h;
    return TextureClip{ this->texture, tx, ty, tw, th };
}


std::shared_ptr<CellImage> TextureClip::clip(short i, short j)
{
    return std::make_shared<TextureClip>(clip(i * 0.5f, j * 0.5f, 0.5f, 0.5f));
}


int TextureClip::getRecalcPriority() const
{
    return int(1.0f / tw);
}


QuadImage::~QuadImage(void)
{
}


void QuadImage::drawRect(float x, float y, float width, float height)
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            this->cells[i][j]->drawRect(x + i * 0.5f * width,
                                        y + j * 0.5f * height,
                                        width * 0.5f,
                                        height * 0.5f);
        }
    }
}


std::shared_ptr<CellImage> QuadImage::clip(short i, short j)
{
    return cells[i][j];
}


int QuadImage::getRecalcPriority() const
{
    return 1;
}

TexGrid::TexGrid(MandelView& owner, int level) :
    owner{ owner },
    level{ level },
    dpp{ owner.getDpp(level) }
{
}


std::pair<GridIndex, GridIndex> TexGrid::getCellIndices(mnd::Real x, mnd::Real y)
{
    return { GridIndex(mnd::floor(x / dpp / MandelView::chunkSize)), GridIndex(mnd::floor(y / dpp / MandelView::chunkSize)) };
}


std::pair<mnd::Real, mnd::Real> TexGrid::getPositions(GridIndex x, GridIndex y)
{
    return { mnd::Real(x) * dpp * MandelView::chunkSize, mnd::Real(y) * dpp * MandelView::chunkSize };
}


GridElement* TexGrid::getCell(GridIndex i, GridIndex j)
{
    auto cIt = cells.find({i, j});
    if (cIt != cells.end()) {
        return cIt->second.get();
    }
    else {
        return nullptr;
    }
}


void TexGrid::setCell(GridIndex i, GridIndex j, std::unique_ptr<GridElement> tex)
{
    cells[{i, j}] = std::move(tex);
}


void TexGrid::clearCells(void)
{
    cells.clear();
}


void TexGrid::clearUncleanCells(void)
{
    for (auto it = cells.begin(); it != cells.end();) {
        if (it->second->img->getRecalcPriority() > 1)
            cells.erase(it++);
        else ++it;
    }
}


void Job::run(void)
{
    auto [absX, absY] = grid->getPositions(i, j);
    mnd::Real gw = grid->dpp * MandelView::chunkSize;

    Bitmap<float> f(MandelView::chunkSize, MandelView::chunkSize);
    mnd::MandelInfo mi = owner.getMandelInfo();
    mi.view.x = absX;
    mi.view.y = absY;
    mi.view.width = mi.view.height = gw;
    mi.bWidth = mi.bHeight = MandelView::chunkSize;
    try {
        generator->generate(mi, f.pixels.get());
        auto* rgb = new Bitmap<RGBColor>(f.map<RGBColor>([&mi, this] (float i) {
            return i >= mi.maxIter ? RGBColor{ 0, 0, 0 } : gradient.get(i);
        }));
        emit done(level, i, j, calcState, rgb);
    }
    catch(std::exception& ex) {
        printf("wat: %s?!\n", ex.what()); fflush(stdout);
        exit(1);
    }
    catch(...) {
        printf("wat?!\n"); fflush(stdout);
        exit(1);
    }
}


Calcer::Calcer(mnd::MandelGenerator* generator, MandelWidget& owner) :
    jobsMutex{ QMutex::Recursive },
    generator{ generator },
    threadPool{ std::make_unique<QThreadPool>() },
    owner{ owner },
    gradient{ owner.getGradient() }
{
    threadPool->setMaxThreadCount(1);
}


void Calcer::clearAll(void)
{
    this->threadPool->clear();
}


void Calcer::calc(TexGrid& grid, int level, GridIndex i, GridIndex j, int priority)
{
    jobsMutex.lock();
    if (jobs.find({ level, i, j }) == jobs.end()) {
        Job* job = new Job(generator, gradient, owner, &grid, level, i, j, calcState);
        connect(job, &Job::done, this, &Calcer::redirect);
        connect(job, &QObject::destroyed, this, [this, level, i, j] () { this->notFinished(level, i, j); });
        jobs.emplace(std::tuple{level, i, j}, job);
        threadPool->start(job, priority);
    }
    jobsMutex.unlock();
}


void Calcer::setCurrentLevel(int level)
{
    if (this->currentLevel != level) {
        this->currentLevel = level;
        std::vector<QRunnable*> toCancel;
        jobsMutex.lock();
        for (auto&[tup, job] : jobs) {
            auto& [level, i, j] = tup;
            if(level != currentLevel) {
                toCancel.push_back(job);
            }
        }
        jobsMutex.unlock();
        for (auto* job : toCancel) {
            if (threadPool->tryTake(job)) {
                delete job;
            }
        }
    }
}


void Calcer::notFinished(int level, GridIndex i, GridIndex j)
{
    jobsMutex.lock();
    jobs.erase({ level, i, j });
    jobsMutex.unlock();
}


void Calcer::redirect(int level, GridIndex i, GridIndex j, long calcState, Bitmap<RGBColor>* bmp)
{
    jobsMutex.lock();
    jobs.erase({ level, i, j });
    jobsMutex.unlock();
    if (this->calcState == calcState) {
        emit done(level, i, j, bmp);
    }
    else {
        delete bmp;
    }
}


const int MandelView::chunkSize = 256;

MandelView::MandelView(mnd::MandelGenerator* generator, MandelWidget& owner) :
    generator{ generator },
    calcer{ generator, owner },
    owner{ owner }
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
    connect(&calcer, &Calcer::done, this, &MandelView::cellReady);
}


int MandelView::getLevel(mnd::Real dpp) {
    return int(mnd::log2(dpp / chunkSize));
}


mnd::Real MandelView::getDpp(int level)
{
    return mnd::pow(mnd::Real(2), mnd::Real(level)) * chunkSize;
}


TexGrid& MandelView::getGrid(int level)
{
    auto it = levels.find(level);
    if (it != levels.end()) {
        return it->second;
    }
    else {
        levels.insert(std::pair<int, TexGrid>{ level, TexGrid{ *this, level } });
        return levels.at(level);
    }
}


void MandelView::setGenerator(mnd::MandelGenerator* generator)
{
    if (this->generator != generator) {
        this->generator = generator;
        calcer.setGenerator(generator);
        clearCells();
        emit redrawRequested();
    }
}


void MandelView::clearCells(void)
{
    for(auto& [level, grid] : this->levels) {
        grid.clearCells();
    }
}


void MandelView::garbageCollect(int level, GridIndex /*i*/, GridIndex /*j*/)
{
    for(auto& [l, grid] : levels) {
        int dist = ::abs(l - level);

        if (dist == 1) {
            grid.clearUncleanCells();
        }

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


GridElement* MandelView::searchAbove(int level, GridIndex i, GridIndex j, int recursionLevel)
{
    auto& grid = getGrid(level);
    auto& gridAbove = getGrid(level + 1);

    GridIndex ai = (i < 0 ? (i - 1) : i) / 2;
    GridIndex aj = (j < 0 ? (j - 1) : j) / 2;

    GridElement* above = gridAbove.getCell(ai, aj);

    if (above == nullptr && recursionLevel > 0) {
        auto abFound = searchAbove(level + 1, ai, aj, recursionLevel - 1);
        if (abFound)
            above = abFound;
    }

    if (above != nullptr) {
        auto newElement = std::make_unique<GridElement>(
            false, above->img->clip(short(i & 1), short(j & 1))
        );
        GridElement* ret = newElement.get();
        grid.setCell(i, j, std::move(newElement));
        return ret;
    }
    else {
        return nullptr;
    }
}


GridElement* MandelView::searchUnder(int level, GridIndex i, GridIndex j, int recursionLevel)
{
    if (recursionLevel == 0)
        return nullptr;

    auto& grid = getGrid(level);
    auto& gridUnder = getGrid(level - 1);

    GridIndex ai = i * 2;
    GridIndex aj = j * 2;

    GridElement* u00 = gridUnder.getCell(ai, aj);
    GridElement* u01 = gridUnder.getCell(ai, aj + 1);
    GridElement* u10 = gridUnder.getCell(ai + 1, aj);
    GridElement* u11 = gridUnder.getCell(ai + 1, aj + 1);

    /*if (   u00 == nullptr
        || u01 == nullptr
        || u10 == nullptr
        || u11 == nullptr) {
        auto abFound = searchUnder(level + 1, ai, aj, recursionLevel - 1);
        if (abFound)
            above = abFound;
    }*/

    if (   u00 != nullptr
        && u01 != nullptr
        && u10 != nullptr
        && u11 != nullptr) {
        auto newElement = std::make_unique<GridElement>(
            false, std::make_shared<QuadImage>(u00->img, u01->img, u10->img, u11->img)
        );
        GridElement* ret = newElement.get();
        grid.setCell(i, j, std::move(newElement));
        return ret;
    }
    else {
        return nullptr;
    }
}


void MandelView::paint(const mnd::MandelViewport& mvp)
{
    mnd::Real dpp = mvp.width / width;
    int level = getLevel(dpp) - 1;
    auto& grid = getGrid(level);
    mnd::Real gw = getDpp(level) * chunkSize;
    auto [left, top] = grid.getCellIndices(mvp.x, mvp.y);
    auto [right, bottom] = grid.getCellIndices(mvp.right(), mvp.bottom());

    garbageCollect(level, (left + right) / 2, (top + bottom) / 2);
    emit calcer.setCurrentLevel(level);

    mnd::Real w = width * gw / mvp.width;

    auto [realXLeft, realYTop] = grid.getPositions(left, top);
    realXLeft = ((realXLeft - mvp.x) * mnd::Real(width)) / mvp.width;
    realYTop = ((realYTop - mvp.y) * mnd::Real(height)) / mvp.height;
    for(GridIndex i = left; i <= right; i++) {
        for(GridIndex j = top; j <= bottom; j++) {
            mnd::Real x = w * int(i - left) + realXLeft;
            mnd::Real y = w * int(j - top) + realYTop;

            GridElement* t = grid.getCell(i, j);

            if (t == nullptr) {
                auto under = searchUnder(level, i, j, 1);
                if (under) {
                    t = under;
                }
                else {
                    auto above = searchAbove(level, i, j, 3);
                    if (above) {
                        t = above;
                    }
                }
            }

            if (t != nullptr) {
                t->img->drawRect(float(x), float(y), float(w), float(w));
                /*glBegin(GL_LINE_LOOP);
                glVertex2f(float(x), float(y));
                glVertex2f(float(x) + float(w), float(y));
                glVertex2f(float(x) + float(w), float(y) + float(w));
                glVertex2f(float(x), float(y) + float(w));
                glEnd();*/

                if (!t->enoughResolution) {
                    calcer.calc(grid, level, i, j, t->img->getRecalcPriority());
                }
            }
            else {
                calcer.calc(grid, level, i, j, 1000);
                this->empty->drawRect(float(x), float(y), float(w), float(w));
            }
        }
    }
}

void MandelView::cellReady(int level, GridIndex i, GridIndex j, Bitmap<RGBColor>* bmp)
{

    this->getGrid(level).setCell(i, j,
        std::make_unique<GridElement>(true, std::make_shared<TextureClip>(std::make_shared<Texture>(*bmp))));
    delete bmp;
    emit redrawRequested();
}


MandelWidget::MandelWidget(mnd::MandelContext& ctxt, mnd::MandelGenerator* generator, QWidget* parent) :
    QOpenGLWidget{ parent },
    mndContext{ ctxt },
    generator{ generator },
    gradient{ Gradient::defaultGradient() }
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setSizePolicy(QSizePolicy::Expanding,
        QSizePolicy::Expanding);
    qRegisterMetaType<GridIndex>("GridIndex");
    this->format().setSwapInterval(1);

    /*gradient = Gradient {
        {
            { RGBColor{ 0, 0, 0 }, 0 },
            { RGBColor{ 180, 20, 10 }, 30 },
            { RGBColor{ 210, 180, 15 }, 70 },
            { RGBColor{ 160, 220, 45 }, 170 },
            { RGBColor{ 50, 150, 170 }, 300 },
        }
    };*/
}


MandelWidget::~MandelWidget()
{
}


void MandelWidget::setGradient(Gradient g)
{
    this->gradient = std::move(g);
    if (mandelView) {
        mandelView->clearCells();
        mandelView->calcer.changeState();
    }
    emit update();
}


void MandelWidget::setSmoothColoring(bool sc)
{
    if (sc != mandelInfo.smooth) {
        mandelInfo.smooth = sc;
        if (mandelView) {
            mandelView->clearCells();
            emit update();
        }
    }
}


void MandelWidget::setDisplayInfo(bool di)
{
    if (di != this->displayInfo) {
        this->displayInfo = di;
        emit update();
    }
}


void MandelWidget::setMaxIterations(int maxIter)
{
    if (mandelInfo.maxIter != maxIter) {
        mandelInfo.maxIter = maxIter;
        if (mandelView) {
            mandelView->clearCells();
            mandelView->calcer.clearAll();
            mandelView->calcer.changeState();
        }
        emit update();
    }
}


void MandelWidget::setJuliaPos(const mnd::Real& x, const mnd::Real& y)
{
    mandelInfo.juliaX = x;
    mandelInfo.juliaY = y;
    if (mandelView)
        mandelView->calcer.changeState();
    emit update();
}


void MandelWidget::setGenerator(mnd::MandelGenerator* generator)
{
    if (this->generator != generator) {
        this->generator = generator;
        if (mandelView)
            mandelView->setGenerator(generator);
    }
}


void MandelWidget::clearAll(void)
{
    mandelView->clearCells();
    mandelView->calcer.clearAll();
}


void MandelWidget::initializeGL(void)
{
    this->context()->functions()->glClearColor(0, 0, 0, 0);
    this->context()->makeCurrent(nullptr);

    glDisable(GL_DEPTH_TEST);

    // looks not even better
    glEnable(GL_FRAMEBUFFER_SRGB);

    //glShadeModel(GL_SMOOTH);

    mandelView = nullptr;
    requestRecalc();
}


void MandelWidget::paintGL(void)
{
    if (mandelView == nullptr) {
        mandelView = std::make_unique<MandelView>(generator, *this);
        QObject::connect(mandelView.get(), &MandelView::redrawRequested, this, static_cast<void(QOpenGLWidget::*)(void)>(&QOpenGLWidget::update));
    }

    int width = this->width();
    int height = this->height();
    mandelView->width = width;
    mandelView->height = height;

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

    updateAnimations();
    mandelView->paint(this->currentViewport);

    if (rubberbanding)
        drawRubberband();
    if (displayInfo)
        drawInfo();
    if (selectingPoint)
        drawPoint();
}


void MandelWidget::updateAnimations(void)
{
    if (mnd::abs(currentViewport.width / targetViewport.width - 1.0) < 0.1e-5
            && mnd::abs(currentViewport.height / targetViewport.height - 1.0) < 0.1e-5) {
        // animation finished
        currentViewport = targetViewport;
    }
    else {
        auto now = std::chrono::high_resolution_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimUpdate).count();
        const mnd::Real factor = mnd::Real(::pow(0.97, millis));
        const mnd::Real one(1.0);

        currentViewport.x = currentViewport.x * factor + targetViewport.x * (one - factor);
        currentViewport.y = currentViewport.y * factor + targetViewport.y * (one - factor);
        currentViewport.width = currentViewport.width * factor + targetViewport.width * (one - factor);
        currentViewport.height = currentViewport.height * factor + targetViewport.height * (one - factor);

        lastAnimUpdate = now;
        emit update();
    }

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.9f, 0.1f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(rubberband.x(), rubberband.y());
    glVertex2d(rubberband.right(), rubberband.y());
    glVertex2d(rubberband.right(), rubberband.bottom());
    glVertex2d(rubberband.x(), rubberband.bottom());
    glEnd();
    glDisable(GL_BLEND);
}


void MandelWidget::drawInfo(void)
{
    const float DIST_FROM_BORDER = 15;
    float maxWidth = this->width() - 2 * DIST_FROM_BORDER;
    mnd::Real distPerPixel = currentViewport.width / this->width();
    float log10 = (mnd::convert<float>(mnd::log(distPerPixel)) + ::logf(maxWidth)) / ::logf(10);
    mnd::Real displayDist = mnd::pow(mnd::Real(10), ::floor(log10));
    float pixels = mnd::convert<float>(displayDist / distPerPixel);
    int factor = 1;
    for (int i = 9; i > 1; i--) {
        if (pixels * i < maxWidth) {
            factor *= i;
            pixels *= i;
            displayDist *= i;
            break;
        }
    }

    std::stringstream dis;
    if (::abs(log10) < 3) {
        dis << mnd::convert<float>(displayDist);
    }
    else {
        dis << factor << "e" << int(::floor(log10));
    }

    if (maxWidth > 400) {
        dis << "; per pixel: " << distPerPixel;
    }

    float lineY = this->height() - DIST_FROM_BORDER;
    float lineXEnd = DIST_FROM_BORDER + pixels;

    QPainter infoPainter{ this };
    infoPainter.setPen(Qt::white);
    infoPainter.setFont(QFont("Arial", 12));
    infoPainter.drawLine(QPointF{ DIST_FROM_BORDER, lineY }, QPointF{ lineXEnd, lineY });
    infoPainter.drawLine(QPointF{ DIST_FROM_BORDER, lineY }, QPointF{ DIST_FROM_BORDER, lineY - 5 });
    infoPainter.drawLine(QPointF{ lineXEnd, lineY }, QPointF{ lineXEnd, lineY - 5 });
    infoPainter.drawText(int(DIST_FROM_BORDER), int(lineY - 20), int(lineXEnd - DIST_FROM_BORDER), 20,
                         Qt::AlignCenter, QString::fromStdString(dis.str()));
    infoPainter.end();
}


void MandelWidget::drawPoint(void)
{
    glColor3ub(255, 255, 255);
    glBegin(GL_LINES);
    glVertex2f(0, pointY);
    glVertex2f(width(), pointY);

    glVertex2f(pointX, 0);
    glVertex2f(pointX, height());
    glEnd();
}


void MandelWidget::zoom(float scale, float x, float y)
{
    targetViewport.zoom(scale, x, y);
    lastAnimUpdate = std::chrono::high_resolution_clock::now();
    //currentViewport.zoom(scale, x, y);
    requestRecalc();
}


void MandelWidget::setViewport(const mnd::MandelViewport& viewport)
{
    targetViewport = viewport;
    targetViewport.adjustAspectRatio(this->width(), this->height());
    currentViewport = targetViewport;
    //lastAnimUpdate = std::chrono::high_resolution_clock::now();
    //currentViewport.zoom(scale, x, y);
    requestRecalc();
}


void MandelWidget::selectPoint(void)
{
    this->selectingPoint = true;
    this->setMouseTracking(true);
}


void MandelWidget::stopSelectingPoint(void)
{
    this->selectingPoint = false;
    this->setMouseTracking(false);
}


void MandelWidget::requestRecalc()
{
    emit update();
}


void MandelWidget::resizeEvent(QResizeEvent* re)
{
    QOpenGLWidget::resizeEvent(re);
    double aspect = double(geometry().width()) / geometry().height();

    currentViewport.height = currentViewport.width / aspect;
    targetViewport = currentViewport;

    if (mandelView.get() != nullptr) {
        mandelView->width = this->width();
        mandelView->height = this->height();
    }
    requestRecalc();
}


void MandelWidget::mousePressEvent(QMouseEvent* me)
{
    QOpenGLWidget::mousePressEvent(me);
    if (me->button() == Qt::RightButton) {
        rubberbanding = true;
        rubberband.setCoords(me->x(), me->y(), me->x(), me->y());
        emit repaint();
        me->accept();
    }
    else if (me->button() == Qt::LeftButton) {
        dragging = true;
        dragX = me->x();
        dragY = me->y();
        me->accept();
    }
}


void MandelWidget::mouseMoveEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseMoveEvent(me);

    if (rubberbanding) {
        QRectF& rect = rubberband;
        double aspect = double(geometry().width()) / geometry().height();
        rect.setBottomRight(QPoint(me->x(), me->y()));
        if (rect.width() > rect.height() * aspect)
            rect.setHeight(rect.width() / aspect);
        else
            rect.setWidth(rect.height() * aspect);

        emit repaint();
    }
    else if (selectingPoint) {
        pointX = me->x();
        pointY = me->y();
        emit repaint();
    }
    else if (dragging) {
        double deltaX = me->x() - dragX;
        double deltaY = me->y() - dragY;

        this->currentViewport.x -= deltaX * currentViewport.width / this->width();
        this->currentViewport.y -= deltaY * currentViewport.height / this->height();
        targetViewport = currentViewport;
        dragX = me->x(); dragY = me->y();

        emit repaint();
    }
    me->accept();
}


void MandelWidget::mouseReleaseEvent(QMouseEvent* me)
{
    QOpenGLWidget::mouseReleaseEvent(me);
    if (rubberbanding) {
        QRect rect = rubberband.toRect();
        if(rect.width() != 0 && rect.height() != 0) {
            QRect full = this->geometry();

            targetViewport.x += mnd::Real(rect.left()) * targetViewport.width / full.width();
            targetViewport.y += mnd::Real(rect.top()) * targetViewport.height / full.height();
            targetViewport.width *= mnd::Real(rect.width()) / full.width();
            targetViewport.height *= mnd::Real(rect.height()) / full.height();
            targetViewport.normalize();
            currentViewport = targetViewport;
        }
        requestRecalc();
        rubberbanding = false;
    }
    else if (selectingPoint) {
        selectingPoint = false;
        this->setMouseTracking(false);
        mnd::Real x = currentViewport.x + currentViewport.width * mnd::convert<mnd::Real>(float(me->x()) / width());
        mnd::Real y = currentViewport.y + currentViewport.height * mnd::convert<mnd::Real>(float(me->y()) / height());
        emit pointSelected(x, y);
        emit repaint();
    }
    dragging = false;

    //requestRecalc();
}

void MandelWidget::wheelEvent(QWheelEvent* we)
{
    QOpenGLWidget::wheelEvent(we);
    float x = float(we->x()) / this->width();
    float y = float(we->y()) / this->height();
    float scale = ::powf(0.9975f, we->angleDelta().y());
    zoom(scale, x, y);
    if (!we->pixelDelta().isNull())
        this->currentViewport = this->targetViewport;
    we->accept();
}


/*void MandelWidget::viewUpdated(Bitmap<RGBColor>* bitmap)
{
    if (bitmap != nullptr) {
        delete bitmap;
        emit repaint();
    }
}*/
