#ifndef FRACTALWIDGETUTILS_H
#define FRACTALWIDGETUTILS_H

#include "EscapeTimeVisualWidget.h"
#include "Mandel.h"
#include "Bitmap.h"
#include <unordered_map>
#include <QMetaType>
#include <QObject>
#include <QRunnable>

#include <utility>


class SliceGrid;
class FractalZoomWidget;


using GridIndex = mnd::Integer;
Q_DECLARE_METATYPE(GridIndex)
Q_DECLARE_METATYPE(mnd::Real)


class CellImage
{
public:
    CellImage(void) = default;
    CellImage(CellImage&& b) = default;
    CellImage(const CellImage& b) = delete;
    virtual ~CellImage(void);

    virtual void drawRect(float x, float y, float width, float height) = 0;
    virtual std::shared_ptr<CellImage> clip(short i, short j) = 0;
    virtual int getRecalcPriority(void) const = 0;
};


class ImageClip :
    public CellImage
{
    std::shared_ptr<ETVImage> etvImage;
    float tx, ty, tw, th;
public:
    ImageClip(std::shared_ptr<ETVImage> tex,
              float tx, float ty, float tw, float th);

    inline ImageClip(std::shared_ptr<ETVImage> tex) :
        ImageClip{ tex, 0.0f, 0.0f, 1.0f, 1.0f }
    {}

    ImageClip(ImageClip&&) = default;
    ImageClip(const ImageClip&) = delete;

    virtual ~ImageClip(void);

    void drawRect(float x, float y, float width, float height) override;

    ImageClip clip(float x, float y, float w, float h);
    std::shared_ptr<CellImage> clip(short i, short j) override;
    int getRecalcPriority(void) const override;
};


class QuadImage :
    public CellImage
{
    std::shared_ptr<CellImage> cells[2][2];
public:
    QuadImage(std::shared_ptr<CellImage> i00,
              std::shared_ptr<CellImage> i01,
              std::shared_ptr<CellImage> i10,
              std::shared_ptr<CellImage> i11);

    QuadImage(QuadImage&&) = default;
    QuadImage(const QuadImage&) = delete;

    virtual ~QuadImage(void);

    void drawRect(float x, float y, float width, float height) override;
    std::shared_ptr<CellImage> clip(short i, short j) override;
    int getRecalcPriority(void) const override;
};


struct GridElement
{
    bool enoughResolution;
    std::shared_ptr<CellImage> img;
    inline GridElement(bool enoughResolution, std::shared_ptr<CellImage> img) :
        enoughResolution{ enoughResolution },
        img{ std::move(img) }
    {}
};


class CalcJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    FractalZoomWidget& owner;
    mnd::MandelGenerator* generator;
    SliceGrid* grid;
    int level;
    GridIndex i, j;
    long calcState = 0;

    inline CalcJob(FractalZoomWidget& owner,
        mnd::MandelGenerator* generator,
        SliceGrid* grid, int level,
        GridIndex i, GridIndex j,
        long calcState) :
        owner{ owner },
        generator{ generator },
        grid{ grid },
        level{ level },
        i{ i }, j{ j },
        calcState{ calcState }
    {}

    void run() override;
signals:
    void done(int level, GridIndex i, GridIndex j, Bitmap<float>* bmp);
    void failed(int level, GridIndex i, GridIndex j, QString err);
};


struct IndexPairHash
{
    size_t operator()(const std::pair<GridIndex, GridIndex>& p) const;
};


struct IndexTripleHash
{
    size_t operator()(const std::tuple<int, GridIndex, GridIndex>& p) const;
};

#endif // FRACTALWIDGETUTILS_H
