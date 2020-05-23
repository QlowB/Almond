#include "FractalWidgetUtils.h"


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


void QuadImage::drawRect(QOpenGLShaderProgram* program,
        float x, float y, float width, float height)
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            this->cells[i][j]->drawRect(program,
                                        x + i * 0.5f * width,
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
