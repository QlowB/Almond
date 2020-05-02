#include "Almond.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>
#include <QGradient>
#include "gradientchoosedialog.h"

#include <cmath>

Almond::Almond(QWidget* parent) :
    QMainWindow{ parent },
    mandelContext{ mnd::initializeContext() }
{
    ui.setupUi(this);
    mw = std::make_unique<MandelWidget>(mandelContext,
                                        &mandelContext.getDefaultGenerator(),
                                        ui.centralWidget);
    customGeneratorDialog = std::make_unique<CustomGenerator>(mandelContext);
    customGenerator = nullptr;
    customViewSave = mnd::MandelViewport::centerView();

    on_maxIterations_editingFinished();
    mw->setSmoothColoring(ui.smooth->isChecked());


    currentView = MANDELBROT;
    mandelGenerator = &mandelContext.getDefaultGenerator();
    mandelViewSave = mw->getViewport();

    QObject::connect(mw.get(), &MandelWidget::pointSelected, this, &Almond::pointSelected);
    ui.mainContainer->addWidget(mw.get());
    ui.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
    ui.backgroundProgress->setVisible(false);
    //ui.verticalLayout_left->addWidget(new MyGLWidget(ui.centralWidget));
    //mw->show();
}


Almond::~Almond(void)
{
}


void Almond::submitBackgroundTask(BackgroundTask* task)
{
    QObject::connect(task, &BackgroundTask::finished, this, &Almond::backgroundTaskFinished);
    QObject::connect(task, &BackgroundTask::progress, this, &Almond::backgroundTaskProgress);
    backgroundTasks.start(task);
    //if (taken) {
        ui.backgroundProgress->setRange(0, 0);
        ui.backgroundProgress->setVisible(true);
        ui.backgroundProgress->setFormat("");
    //}
}


void Almond::backgroundTaskFinished(bool succ)
{
    ui.backgroundProgress->setVisible(false);
    ui.backgroundProgress->setFormat("");
}


void Almond::backgroundTaskProgress(float percentage)
{
    QObject* task = QObject::sender();
    if (auto* bt = qobject_cast<BackgroundTask*>(task)) {
        ui.backgroundProgress->setFormat(QString::fromStdString(bt->getShortDescription() + ": %p%"));
    }
    if (percentage > 0) {
        ui.backgroundProgress->setRange(0, 100);
        ui.backgroundProgress->setValue(percentage);
    }
    else {
        ui.backgroundProgress->setRange(0, 0);
    }
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
    gcd.exec();
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
        MandelVideoGenerator mvg(evi);
        mnd::MandelGenerator& g = *mw->getGenerator();
        submitBackgroundTask(new VideoExportTask(std::move(mvg), g));
        //if (exportVideo(evi)) {

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


void Almond::on_smooth_stateChanged(int checked)
{
    this->mw->setSmoothColoring(checked != Qt::Unchecked);
}


void Almond::on_exportImage_clicked()
{
    ExportImageDialog dialog(this);
    dialog.setMaxIterations(mw->getMaxIterations());
    //dialog.show();
    auto response = dialog.exec();
    if (response == 1) {
        mnd::MandelInfo mi;
        mi.maxIter = dialog.getMaxIterations();
        mi.view = mw->getViewport();
        mi.bWidth = dialog.getWidth();
        mi.bHeight = dialog.getHeight();
        mi.view.adjustAspectRatio(mi.bWidth, mi.bHeight);
        mi.smooth = mw->getSmoothColoring();
        if (currentView == JULIA) {
            mi.julia = mw->getMandelInfo().julia;
            mi.juliaX = mw->getJuliaX();
            mi.juliaY = mw->getJuliaY();
        }
        mnd::MandelGenerator* currentGenerator = mw->getGenerator();
        mnd::MandelGenerator& g = currentGenerator ? *currentGenerator : mandelContext.getDefaultGenerator();

        alm::ImageExportInfo iei;
        iei.drawInfo = mi;
        iei.generator = &g;
        iei.gradient = mw->getGradient();
        iei.path = dialog.getPath().toStdString();
        submitBackgroundTask(new ImageExportTask(iei));

        /*auto exprt = [iei, path = dialog.getPath().toStdString()]() {
            alm::exportPng(path, iei);
        };

        submitBackgroundTask();*/
        
        /*auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
        g.generate(mi, fmap.pixels.get());
        auto bitmap = fmap.map<RGBColor>([&mi, this] (float i) {
            return i >= mi.maxIter ? RGBColor{ 0,0,0 } : mw->getGradient().get(i);
        });
        QImage img(reinterpret_cast<unsigned char*>(bitmap.pixels.get()), bitmap.width, bitmap.height, bitmap.width * 3, QImage::Format_RGB888);
        img.save(dialog.getPath());*/
    }
}


void Almond::on_resetZoom_clicked()
{
    if (currentView == MANDELBROT) {
        mw->setViewport(mnd::MandelViewport::standardView());
    }
    else {
        mw->setViewport(mnd::MandelViewport::centerView());
    }
}


void Almond::on_displayInfo_stateChanged(int checked)
{
    this->mw->setDisplayInfo(checked != Qt::Unchecked);
}


void Almond::on_chooseGenerator_clicked()
{
    std::unique_ptr<ChooseGenerators> generatorsDialog;
    if (currentView == MANDELBROT || currentView == JULIA)
        generatorsDialog = std::make_unique<ChooseGenerators>(mandelContext, *mandelGenerator, *this);
    else if (currentView == CUSTOM)
        generatorsDialog = std::make_unique<ChooseGenerators>(mandelContext, this->currentCustom->gc, *customGenerator, *this);
    else
        return;

    auto response = generatorsDialog->exec();

    auto gen = generatorsDialog->extractChosenGenerator();
    if (gen) {
        if (currentView == MANDELBROT || currentView == JULIA) {
            mandelGenerator = gen.get();
        }
        else if (currentView == CUSTOM) {
            customGenerator = gen.get();
        }
        currentGenerator = gen.get();
        this->mw->setGenerator(currentGenerator);
        adjustedGenerators.push_back(std::move(gen));
    }
    else {
        //mandelGenerator = &mandelContext.getDefaultGenerator();
    }
    //this->currentView = MANDELBROT;
    //this->mw->getMandelInfo().julia = false;
    //printf("dialog executed\n"); fflush(stdout);
}


void Almond::pointSelected(mnd::Real x, mnd::Real y)
{
    if (currentView != JULIA) {
        saveView();
        this->mw->setViewport(mnd::MandelViewport::centerView());
        this->mw->setJuliaPos(x, y);
        this->mw->getMandelInfo().julia = true;
        this->mw->clearAll();
    }
    currentView = JULIA;
}


void Almond::on_groupBox_toggled(bool arg1)
{
    printf("arg1: %i\n", int(arg1)); fflush(stdout);
}

void Almond::on_wMandel_clicked()
{

}


void Almond::saveView()
{
    if (currentView == MANDELBROT)
        mandelViewSave = mw->getViewport();
    else if (currentView == CUSTOM)
        customViewSave = mw->getViewport();
}


void Almond::setViewType(ViewType v)
{
    saveView();
    if (v == MANDELBROT) {
        currentGenerator = mandelGenerator;
        emit this->mw->stopSelectingPoint();
        this->mw->setViewport(mandelViewSave);
        this->mw->setGenerator(currentGenerator);
        this->mw->getMandelInfo().julia = false;
        this->mw->clearAll();
        currentView = MANDELBROT;
    }
    else if (v == CUSTOM) {
        if (customGenerator != nullptr) {
            currentGenerator = customGenerator;
            this->mw->setGenerator(currentGenerator);
            emit this->mw->stopSelectingPoint();
            this->mw->setViewport(customViewSave);
            this->mw->getMandelInfo().julia = false;
            this->mw->clearAll();
            currentView = CUSTOM;
        }
        else {
            setViewType(MANDELBROT);
        }
    }
    else if (v == JULIA) {
        if (currentView == MANDELBROT) {
            emit this->mw->selectPoint();
        }
        else {
            currentView = MANDELBROT;
            currentGenerator = mandelGenerator;
            this->mw->setGenerator(currentGenerator);
            this->mw->setViewport(mandelViewSave);
            this->mw->getMandelInfo().julia = false;
            this->mw->clearAll();
            emit this->mw->selectPoint();
        }
    }
}


void Almond::on_wMandel_toggled(bool checked)
{
    if (checked)
        setViewType(MANDELBROT);
}

void Almond::on_radioButton_toggled(bool checked)
{
    saveView();
    if (checked) {
        setViewType(JULIA);
    }
}

void Almond::on_radioButton_2_toggled(bool checked)
{
    saveView();
    if (checked) {
        if (customGenerator == nullptr) {
            customGeneratorDialog->exec();
            if (auto* frac = customGeneratorDialog->getLastCompiled()) {
                customGenerator = frac->gc.adaptiveGenerator.get();
                customGenerators.push_back(std::make_unique<FractalDef>(std::move(*frac)));
                currentCustom = customGenerators[customGenerators.size() - 1].get();
            }
        }
        setViewType(CUSTOM);
    }
}

void Almond::on_createCustom_clicked()
{
    auto response = customGeneratorDialog->exec();
    if (response != 1)
        return;
    if (auto* frac = customGeneratorDialog->getLastCompiled()) {
        customGenerator = frac->gc.adaptiveGenerator.get();
        customGenerators.push_back(std::make_unique<FractalDef>(std::move(*frac)));
        currentCustom = customGenerators[customGenerators.size() - 1].get();
        this->ui.radioButton_2->setChecked(true);
        setViewType(CUSTOM);
    }
}
