#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>
#include "ui_Almond.h"
#include "ui_exportimagedialog.h"

#include <Mandel.h>
#include "MandelWidget.h"

#include <memory>

class Almond : public QMainWindow
{
    Q_OBJECT
private:
    mnd::MandelContext mandelContext;
    std::unique_ptr<MandelWidget> mw;
public:
    Almond(QWidget *parent = Q_NULLPTR);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::AlmondClass ui;
};


class ExportImageDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ExportImageDialog eid;
public:
    ExportImageDialog(QWidget* parent);

    int getMaxIterations(void) const;
    int getWidth(void) const;
    int getHeight(void) const;
    QString getPath(void) const;
private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();
};
