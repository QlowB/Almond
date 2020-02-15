#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Almond.h"

#include <Mandel.h>
#include "MandelWidget.h"
#include "exportdialogs.h"
#include "gradientchoosedialog.h"
#include "benchmarkdialog.h"

#include <memory>

class Almond : public QMainWindow
{
    Q_OBJECT
private:
    mnd::MandelContext mandelContext;
    std::unique_ptr<MandelWidget> mw;
    std::unique_ptr<BenchmarkDialog> benchmarkDialog;
    GradientChooseDialog gcd;
public:
    Almond(QWidget *parent = Q_NULLPTR);

private slots:
    void on_zoom_out_clicked();
    void on_zoom_in_clicked();
    void on_maxIterations_editingFinished();
    void on_chooseGradient_clicked();
    void on_exportVideo_clicked();
    void on_smooth_stateChanged(int arg1);
    void on_runBenchmark_clicked();
    void on_exportImage_clicked();
    void on_resetZoom_clicked();

    void on_displayInfo_stateChanged(int arg1);

private:
    Ui::AlmondClass ui;
};

