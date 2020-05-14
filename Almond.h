#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Almond.h"

#include <Mandel.h>
#include "MandelWidget.h"
#include "BackgroundTask.h"
#include "exportdialogs.h"
#include "gradientchoosedialog.h"
#include "choosegenerators.h"
#include "customgenerator.h"
//#include "benchmarkdialog.h"

#include <memory>

struct ViewState
{
    mnd::MandelViewport viewport;
    mnd::MandelGenerator* generator;
};


enum ViewType
{
    MANDELBROT,
    JULIA,
    CUSTOM,
};


class Almond : public QMainWindow
{
    Q_OBJECT
private:
    mnd::MandelContext mandelContext;
    QThreadPool backgroundTasks;
    bool stoppingBackgroundTasks = false;


    bool fullscreenMode = false;
    QWidget* cw;
public:
    std::unique_ptr<MandelWidget> mw;
private:
    //std::unique_ptr<BenchmarkDialog> benchmarkDialog;
    std::unique_ptr<CustomGenerator> customGeneratorDialog;
    GradientChooseDialog gcd;

    std::vector<std::unique_ptr<FractalDef>> customGenerators;
    FractalDef* currentCustom;
    mnd::AdaptiveGenerator* customGenerator;
    std::vector<std::unique_ptr<mnd::AdaptiveGenerator>> adjustedGenerators;

    ViewType currentView;
    mnd::MandelViewport mandelViewSave;
    mnd::MandelViewport customViewSave;
    mnd::AdaptiveGenerator* mandelGenerator;
public:
    mnd::MandelGenerator* currentGenerator;
public:
    Almond(QWidget *parent = Q_NULLPTR);
    ~Almond(void);

    void submitBackgroundTask(BackgroundTask* task);
    void stopBackgroundTask();

    bool eventFilter(QObject *target, QEvent *event);
public slots:
    void toggleFullscreen(void);
private slots:
    void on_zoom_out_clicked();
    void on_zoom_in_clicked();
    void on_maxIterations_editingFinished();
    void on_chooseGradient_clicked();
    void on_exportVideo_clicked();
    void on_smooth_stateChanged(int arg1);
    void on_exportImage_clicked();
    void on_resetZoom_clicked();

    void on_displayInfo_stateChanged(int arg1);

    void on_chooseGenerator_clicked();

    void backgroundTaskFinished(bool, QString message);
    void backgroundTaskProgress(float percentage);

    void pointSelected(mnd::Real x, mnd::Real y);

    void on_wMandel_toggled(bool checked);

    void saveView(void);
    void setViewType(ViewType v);

    void on_wMandel_clicked();
    void on_radioButton_toggled(bool checked);
    void on_radioButton_2_toggled(bool checked);
    void on_createCustom_clicked();
    void on_cancelProgress_clicked();

private:
    Ui::AlmondClass ui;
};

