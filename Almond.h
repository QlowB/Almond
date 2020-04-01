#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Almond.h"

#include <Mandel.h>
#include "MandelWidget.h"
#include "BackgroundTask.h"
#include "exportdialogs.h"
#include "gradientchoosedialog.h"
#include "choosegenerators.h"
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
    JULIA
};


class Almond : public QMainWindow
{
    Q_OBJECT
private:
    mnd::MandelContext mandelContext;
    std::unique_ptr<MandelWidget> mw;
    //std::unique_ptr<BenchmarkDialog> benchmarkDialog;
    std::unique_ptr<ChooseGenerators> generatorsDialog;
    GradientChooseDialog gcd;

    std::vector<std::unique_ptr<mnd::MandelGenerator>> compiledGenerators;

    ViewType currentView;
    mnd::MandelViewport mandelViewSave;
    mnd::MandelGenerator* mandelGeneratorSave;

    mnd::MandelGenerator* currentGenerator;
public:
    Almond(QWidget *parent = Q_NULLPTR);
    ~Almond(void);

    void submitBackgroundTask(BackgroundTask* task);

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

    void backgroundTaskFinished();

    void on_selectPoint_clicked();
    void pointSelected(mnd::Real x, mnd::Real y);

    void on_viewMandelbrot_clicked();

private:
    Ui::AlmondClass ui;
};

