#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Almond.h"

#include <Mandel.h>
#include "MandelWidget.h"
#include "exportdialogs.h"
#include "gradientchoosedialog.h"

#include <memory>

class Almond : public QMainWindow
{
    Q_OBJECT
private:
    mnd::MandelContext mandelContext;
    std::unique_ptr<MandelWidget> mw;
    GradientChooseDialog gcd;
public:
    Almond(QWidget *parent = Q_NULLPTR);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_zoom_out_clicked();

    void on_zoom_in_clicked();

    void on_maxIterations_editingFinished();

    void on_chooseGradient_clicked();

    void on_exportVideo_clicked();

private:
    Ui::AlmondClass ui;
};

