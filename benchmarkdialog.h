#ifndef BENCHMARKDIALOG_H
#define BENCHMARKDIALOG_H

#include <QDialog>
#include <functional>
#include "ui_benchmarks.h"
#include <Mandel.h>
#include "Bitmap.h"

class BenchmarkDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::BenchmarkDialog ui;
    mnd::MandelContext& mndContext;
public:
    explicit BenchmarkDialog(mnd::MandelContext& mndContext, QWidget *parent = nullptr);

    mnd::MandelViewport benchViewport(void) const;

    double measureMips(const std::function<Bitmap<float>()>& bench) const;
    QString benchmarkResult(mnd::Generator& mg, int size, int iters) const;

signals:

public slots:
private slots:
    void on_run_clicked();
};

#endif // BENCHMARKDIALOG_H
