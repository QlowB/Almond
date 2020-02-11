#include "Almond.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>
#include <QGradient>
#include "benchmarkdialog.h"
#include "gradientchoosedialog.h"

#include <cmath>

Almond::Almond(QWidget *parent) :
    QMainWindow(parent),
    mandelContext(mnd::initializeContext())
{
    ui.setupUi(this);
    printf("not yet created!\n");
    mw = std::make_unique<MandelWidget>(mandelContext, ui.centralWidget);
    //qRegisterMetaType<MandelWidget>("MandelWidget");
    printf("created!\n");
    ui.verticalLayout_left->addWidget(mw.get());
    ui.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
    //ui.verticalLayout_left->addWidget(new MyGLWidget(ui.centralWidget));
    //mw->show();
}

void Almond::on_pushButton_clicked()
{
    ExportImageDialog dialog(this);
    //dialog.show();
    auto response = dialog.exec();
    if (response == 1) {
        mnd::MandelInfo mi;
        mi.maxIter = dialog.getMaxIterations();
        mi.view = mw->getViewport();
        mi.bWidth = dialog.getWidth();
        mi.bHeight = dialog.getHeight();
        mi.view.adjustAspectRatio(mi.bWidth, mi.bHeight);
        mnd::Generator& g = mandelContext.getCpuGeneratorFloat();
        auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
        g.generate(mi, fmap.pixels.get());
        auto bitmap = fmap.map<RGBColor>([&mi, this] (float i) {
            return i >= mi.maxIter ? RGBColor{ 0,0,0 } : mw->getGradient().get(i);
        });
        QImage img((unsigned char*) bitmap.pixels.get(), bitmap.width, bitmap.height, bitmap.width * 3, QImage::Format_RGB888);
        img.save(dialog.getPath());
    }
}


void Almond::on_pushButton_2_clicked()
{
    BenchmarkDialog bd(mandelContext, this);
    //bd.show();
    bd.exec();
}

void Almond::on_zoom_out_clicked()
{
    mw->zoom(2);
}

void Almond::on_zoom_in_clicked()
{
    mw->zoom(0.5);
}

void Almond::on_maxIterations_editingFinished()
{
    QString text = ui.maxIterations->text();
    int maxIter = text.toInt();
    mw->setMaxIterations(maxIter);
}

void Almond::on_chooseGradient_clicked()
{
    auto response = gcd.exec();
    auto gradient = gcd.getGradient();
    if (gradient)
        mw->setGradient(std::move(*gradient));
}

void Almond::on_exportVideo_clicked()
{
    ExportVideoInfo evi;
    evi.start = mnd::MandelViewport::standardView();
    evi.end = mw->getViewport();
    evi.gradient = mw->getGradient();
    ExportVideoDialog dialog(this, evi);
    //dialog.show();
    auto response = dialog.exec();
    printf("dialog executed\n"); fflush(stdout);
    if (response == 1) {
        mnd::MandelInfo mi;
        evi = dialog.getExportVideoInfo();
        //Video
        /*mi.maxIter = dialog.getMaxIterations();
        mi.view = mw->getViewport();
        mi.bWidth = dialog.getWidth();
        mi.bHeight = dialog.getHeight();
        mi.view.adjustAspectRatio(mi.bWidth, mi.bHeight);
        mnd::Generator& g = mandelContext.getDefaultGenerator();
        auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
        g.generate(mi, fmap.pixels.get());
        auto bitmap = fmap.map<RGBColor>([](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
        QImage img((unsigned char*)bitmap.pixels.get(), bitmap.width, bitmap.height, bitmap.width * 3, QImage::Format_RGB888);
        img.save(dialog.getPath());*/
    }
}
