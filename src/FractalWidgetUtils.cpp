#include "FractalWidgetUtils.h"
#include "FractalZoomWidget.h"



CellImage::~CellImage(void)
{
}

ImageClip::ImageClip(std::shared_ptr<ETVImage> tex,
                 float tx, float ty, float tw, float th) :
    etvImage{ std::move(tex) },
    tx{ tx }, ty{ ty }, tw{ tw }, th{ th }
{
}


ImageClip::~ImageClip(void)
{
}


void ImageClip::drawRect(float x, float y, float width, float height)
{
    etvImage->draw(x, y, width, height, tx, ty, tw, th);
}


ImageClip ImageClip::clip(float x, float y, float w, float h)
{
    float tx = this->tx + x * this->tw;
    float ty = this->ty + y * this->th;
    float tw = this->tw * w;
    float th = this->th * h;
    return ImageClip{ this->etvImage, tx, ty, tw, th };
}


std::shared_ptr<CellImage> ImageClip::clip(short i, short j)
{
    return std::make_shared<ImageClip>(clip(i * 0.5f, j * 0.5f, 0.5f, 0.5f));
}


int ImageClip::getRecalcPriority() const
{
    return int(1.0f / tw);
}


QuadImage::QuadImage(std::shared_ptr<CellImage> i00,
                 std::shared_ptr<CellImage> i01,
                 std::shared_ptr<CellImage> i10,
                 std::shared_ptr<CellImage> i11) :
    cells{ { std::move(i00), std::move(i01) },
           { std::move(i10), std::move(i11) } }
{
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


void CalcJob::run(void)
{
    auto [absX, absY] = grid->getPositions(i, j);
    mnd::Real gw = grid->dpp * FractalZoomWidget::chunkSize;

    Bitmap<float> f{ FractalZoomWidget::chunkSize, FractalZoomWidget::chunkSize };
    mnd::MandelInfo mi = owner.getMandelInfo();
    mi.view.x = absX;
    mi.view.y = absY;
    mi.view.width = mi.view.height = gw;
    mi.bWidth = mi.bHeight = FractalZoomWidget::chunkSize;
    try {
        generator->generate(mi, f.pixels.get());
        emit done(level, i, j, calcState, new Bitmap<float>(std::move(f)));
    }
    catch(std::exception& ex) {
        emit failed(level, i, j, ex.what());
    }
    catch(...) {
        emit failed(level, i, j, tr("unknown error"));
    }
}


size_t IndexPairHash::operator()(const std::pair<GridIndex, GridIndex>& p) const
{
    const auto& [a, b] = p;
    size_t truncA = std::hash<GridIndex>{}(a);
    size_t truncB = std::hash<GridIndex>{}(b);
    boost::hash_combine(truncA, truncB);
    return truncA;
}


size_t IndexTripleHash::operator()(const std::tuple<int, GridIndex, GridIndex>& p) const
{
    const auto& [i, a, b] = p;
    size_t truncA = std::hash<GridIndex>{}(a);
    size_t truncB = std::hash<GridIndex>{}(b);
    boost::hash_combine(truncA, truncB);
    boost::hash_combine(truncA, i);
    return truncA;
}
