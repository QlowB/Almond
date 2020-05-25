#include "Almond.h"
#include <QIntValidator>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QGradient>
#include <QWindow>
#include "gradientchoosedialog.h"

#include "GridFlowLayout.h"

#include <cmath>

Almond::Almond(QWidget* parent) :
    QMainWindow{ parent, Qt::WindowFlags() },
    mandelContext{ mnd::initializeContext() }
{
    ui.setupUi(this);
    fractalWidget = new FractalWidget(this);
    fractalWidget->setGenerator(&mandelContext.getDefaultGenerator());
    fractalWidget->setGradient(Gradient::defaultGradient());
    fractalWidget->setSmoothColoring(ui.smooth->isChecked());

    customGeneratorDialog = std::make_unique<CustomGenerator>(mandelContext);
    customGenerator = nullptr;
    customViewSave = mnd::MandelViewport::centerView();

    on_maxIterations_editingFinished();

    currentView = MANDELBROT;
    mandelGenerator = &mandelContext.getDefaultGenerator();
    // TODO update mandelViewSave = mw->getViewport();

    //QObject::connect(mw.get(), &MandelWidget::pointSelected, this, &Almond::pointSelected);
    ui.mandel_container->addWidget(fractalWidget);
    //ui.mandel_container->addWidget(mw.get());
    ui.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));

    ui.backgroundProgress->setEnabled(false);
    ui.cancelProgress->setEnabled(false);

    amw = new AlmondMenuWidget(this);
    amw->setMainMenu(ui.dockWidgetContents_2);
    eim = new ExportImageMenu();
    evm = new ExportVideoMenu();
    gradientMenu = new GradientMenu();
    AlmondSubMenu* imageSm = amw->addSubMenu(eim);
    AlmondSubMenu* videoSm = amw->addSubMenu(evm);
    AlmondSubMenu* gradientSm = amw->addSubMenu(gradientMenu);
    ui.dockWidget_2->setWidget(amw);

    connect(amw, &AlmondMenuWidget::submenuCancel, [this] (int) { amw->showMainMenu(); });
    //connect(amw, &AlmondMenuWidget::submenuOK, this, &Almond::submenuOK);
    connect(imageSm, &AlmondSubMenu::accepted, this, &Almond::imageExportOk);
    connect(videoSm, &AlmondSubMenu::accepted, this, &Almond::videoExportOk);
    connect(gradientSm, &AlmondSubMenu::accepted, this, &Almond::gradientEditOk);
    connect(gradientSm, &AlmondSubMenu::cancelled, [this] () {
        fractalWidget->setGradient(gradientMenu->getGradientBefore());
    });
    connect(gradientMenu, &GradientMenu::gradientChanged, [this] () {
        fractalWidget->setGradient(gradientMenu->getGradient());
    });


    /*QStatusBar* bar = new QStatusBar(this);
    bar->addWidget(new QLabel("ayay"));
    auto* p = new QPushButton("About");
    bar->addPermanentWidget(p);
    QObject::connect(p, &QPushButton::clicked, [this]() {
        toggleFullscreen();
    });
    bar->setFixedHeight(bar->sizeHint().height());
    //ui.mainContainer->addWidget(bar);
    this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);*/

    installEventFilter(this);

    backgroundTasks.setMaxThreadCount(1);
    QIcon icon{ ":/icons/icon" };
    icon.addFile(":/icons/icon@2x");
    this->setWindowIcon(icon);



    // replace vertical layout with gridflowlayout
    /*GridFlowLayout* gfl = new GridFlowLayout(nullptr);
    //ui.horizontalLayout_4->addItem(gfl);
    for (int i = 0; i < ui.verticalLayout_right->count(); i++) {
        printf("%d: \n", i);
        gfl->addItem(ui.verticalLayout_right->takeAt(i));
    }
    ui.verticalLayout_right->setEnabled(false);
    delete ui.dockWidgetContents_2->layout();
    ui.dockWidgetContents_2->setLayout(gfl);*/
}


Almond::~Almond(void)
{
}


void Almond::submitBackgroundTask(BackgroundTask* task)
{
    QObject::connect(task, &BackgroundTask::finished, this, &Almond::backgroundTaskFinished);
    QObject::connect(task, &BackgroundTask::progress, this, &Almond::backgroundTaskProgress);
    int running = backgroundTasks.activeThreadCount();
    backgroundTasks.start(task);
    if (running == 0) {
        ui.backgroundProgress->setRange(0, 0);
        ui.backgroundProgress->setFormat("");
        ui.backgroundProgress->setEnabled(true);
        ui.cancelProgress->setEnabled(true);
    }
}


void Almond::stopBackgroundTask(void)
{
    stoppingBackgroundTasks = true;
}

bool Almond::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F11) {
            emit toggleFullscreen();
        }
    }
    return QObject::eventFilter(target, event);
}


void Almond::submenuOK(int smIndex)
{
    switch(smIndex) {
    case 0:
        emit imageExportOk();
        break;
    case 1:
        emit videoExportOk();
        break;
    }
}

void Almond::imageExportOk(void)
{
    mnd::MandelInfo mi;
    mi.maxIter = eim->getMaxIterations();
    mi.view = fractalWidget->getViewport();
    mi.bWidth = eim->getWidth();
    mi.bHeight = eim->getHeight();
    mi.view.adjustAspectRatio(mi.bWidth, mi.bHeight);
    mi.smooth = true;
    if (currentView == JULIA) {
        mi.julia = fractalWidget->getMandelInfo().julia;
        mi.juliaX = fractalWidget->getMandelInfo().juliaX;
        mi.juliaY = fractalWidget->getMandelInfo().juliaY;
    }

    mnd::MandelGenerator* currentGenerator = fractalWidget->getGenerator();
    if (currentGenerator == nullptr)
        currentGenerator = &mandelContext.getDefaultGenerator();

    alm::ImageExportInfo iei;
    iei.drawInfo = mi;
    iei.generator = currentGenerator;
    iei.gradient = fractalWidget->getGradient();
    iei.path = eim->getPath().toStdString();
    iei.options.jpegQuality = 95;
    submitBackgroundTask(new ImageExportTask(iei, [this] () { return stoppingBackgroundTasks; }));
    
    amw->showMainMenu();
}


void Almond::videoExportOk(void)
{
    ExportVideoInfo evi = evm->getInfo();
    evi.gradient = fractalWidget->getGradient();
    evi.mi.smooth = fractalWidget->getMandelInfo().smooth;
    if (currentView == JULIA) {
        evi.mi.julia = fractalWidget->getMandelInfo().julia;
        evi.mi.juliaX = fractalWidget->getMandelInfo().juliaX;
        evi.mi.juliaY = fractalWidget->getMandelInfo().juliaY;
    }
    if (evi.path == "") {
        QMessageBox errMsg = QMessageBox(QMessageBox::Icon::Critical, "Error", "No path specified.");
        errMsg.exec();
    }
    else {
        MandelVideoGenerator mvg(evi);
        mnd::MandelGenerator& g = *fractalWidget->getGenerator();
        //printf("wii: %ld\n", evi.mi.bWidth);
        fflush(stdout);
        submitBackgroundTask(new VideoExportTask(std::move(mvg), g));
        amw->showMainMenu();
    }
}


void Almond::gradientEditOk(void)
{
    fractalWidget->setGradient(gradientMenu->getGradient());
    amw->showMainMenu();
}


void Almond::toggleFullscreen(void)
{
    /*
    if (fullscreenMode) {
        auto* m = this->takeCentralWidget();
        ui.mandel_container->addWidget(m);
        this->setCentralWidget(cw);
        emit this->showNormal();
        fullscreenMode = false;
    }
    else {
        cw = this->takeCentralWidget();
        this->setCentralWidget(mw.get());
        emit this->showFullScreen();
        fullscreenMode = true;
    }
    */
}


void Almond::backgroundTaskFinished(bool succ, QString message)
{
    if (succ) {
        QMessageBox info = QMessageBox(QMessageBox::Icon::Information, "Task Finished", message);
        //info->setParent(this);
        emit info.exec();
    }
    else {
        QMessageBox info = QMessageBox(QMessageBox::Icon::Critical, "Task Failed", message);
        //info->setParent(this);
        emit info.exec();
    }

    ui.backgroundProgress->setFormat(tr("Export Progress"));
    ui.backgroundProgress->setEnabled(false);
    ui.backgroundProgress->setRange(0, 100);
    ui.backgroundProgress->setValue(0);
    ui.cancelProgress->setEnabled(false);
    stoppingBackgroundTasks = false;
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
        ui.backgroundProgress->reset();
        ui.backgroundProgress->setRange(0, 0);
        ui.backgroundProgress->setValue(-1);
    }
}


void Almond::on_zoom_out_clicked()
{
    fractalWidget->zoom(2);
}


void Almond::on_zoom_in_clicked()
{
    fractalWidget->zoom(0.5);
}


void Almond::on_maxIterations_editingFinished()
{
    QString text = ui.maxIterations->text();
    int maxIter = text.toInt();
    fractalWidget->setMaxIterations(maxIter);
}


void Almond::on_chooseGradient_clicked()
{
    /*const auto& gradient = fractalWidget->getGradient(); // TODO update
    auto points = gradient.getPoints();
    std::for_each(points.begin(), points.end(), [](auto& x) { x.second /= 300; });

    QVector<QPair<float, QColor>> np;
    std::transform(points.begin(), points.end(), std::back_inserter(np),
        [](auto& qp) -> QPair<float, QColor> {
        auto& [col, pos] = qp;
        return { pos, QColor{ (col.r), (col.g), (col.b) } };
    });*/
    this->gradientMenu->setGradient(fractalWidget->getGradient());
    emit this->amw->showSubMenu(2);
    //gcd.exec();
    //auto gradient = gcd.getGradient();
    //if (gradient)
    //    mw->setGradient(std::move(*gradient));
}


void Almond::on_exportVideo_clicked()
{
    // TODO update
    /*
    evm->setEndViewport(mw->getViewport());
    emit this->amw->showSubMenu(1);
    return;
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
    }*/
}


void Almond::on_smooth_stateChanged(int checked)
{
    fractalWidget->setSmoothColoring(checked != Qt::Unchecked);
}


void Almond::on_exportImage_clicked()
{
    this->amw->showSubMenu(0);
    return;
}


void Almond::on_resetZoom_clicked()
{
    if (currentView == MANDELBROT) {
        fractalWidget->setViewport(mnd::MandelViewport::standardView());
    }
    else {
        fractalWidget->setViewport(mnd::MandelViewport::centerView());
    }
}


void Almond::on_displayInfo_stateChanged(int checked)
{
    this->fractalWidget->setDisplayInfo(checked != Qt::Unchecked);
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
        this->fractalWidget->setGenerator(currentGenerator);
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
        // TODO update
        /*
        this->mw->setViewport(mnd::MandelViewport::centerView());
        this->mw->setJuliaPos(x, y);
        this->mw->getMandelInfo().julia = true;
        this->mw->clearAll();*/
    }
    currentView = JULIA;
}


void Almond::on_wMandel_clicked()
{

}


void Almond::saveView()
{
    if (currentView == MANDELBROT)
        ; // TODO update mandelViewSave = mw->getViewport();
    else if (currentView == CUSTOM)
        ; // TODO update customViewSave = mw->getViewport();
}


void Almond::setViewType(ViewType v)
{
    // TODO update
    /*
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
    */
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


void Almond::on_cancelProgress_clicked()
{
    stopBackgroundTask();
}
