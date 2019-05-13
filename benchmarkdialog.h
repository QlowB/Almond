#ifndef BENCHMARKDIALOG_H
#define BENCHMARKDIALOG_H

#include <QDialog>
#include <functional>
#include "ui_benchmarks.h"
#include "Generators.h"

class BenchmarkDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::BenchmarkDialog ui;
public:
    explicit BenchmarkDialog(QWidget *parent = nullptr);

    MandelViewport benchViewport(void) const;

    double measureMips(const std::function<Bitmap<float>()>& bench) const;
    QString benchmarkResult(MandelGenerator& mg, int size, int iters) const;

signals:

public slots:
private slots:
    void on_run_clicked();
};

#endif // BENCHMARKDIALOG_H
