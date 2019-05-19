#ifndef BENCHMARKDIALOG_H
#define BENCHMARKDIALOG_H

#include <QDialog>
#include <functional>
#include "ui_benchmarks.h"
#include <Mandel.h>
#include "Bitmap.h"
#include <QThread>


struct BenchmarkResult
{
    std::vector<std::vector<double>> values;
    double percentage = 0.0;
};

Q_DECLARE_METATYPE(BenchmarkResult)

class Benchmarker : public QObject
{
    Q_OBJECT
private:
    mnd::MandelContext mndContext;
    static const std::vector<mnd::MandelInfo> benches;
public:
    inline Benchmarker(mnd::MandelContext& mndContext) :
        mndContext{ mnd::initializeContext() }
    {
    }

    static mnd::MandelViewport benchViewport(void);

    std::pair<long long, std::chrono::nanoseconds> measureMips(const std::function<Bitmap<float>()>& bench) const;
    double benchmarkResult(mnd::Generator& mg) const;

public slots:
    void start(void);
signals:
    void update(BenchmarkResult br);
    void finished(void);
};


class BenchmarkDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::BenchmarkDialog ui;
    mnd::MandelContext& mndContext;
    QThread benchThread;
    Benchmarker benchmarker;
public:
    explicit BenchmarkDialog(mnd::MandelContext& mndContext, QWidget *parent = nullptr);


signals:

public slots:
    void update(BenchmarkResult br);
private slots:
    void on_run_clicked();
};

#endif // BENCHMARKDIALOG_H
