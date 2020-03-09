#ifndef CHOOSEGENERATORS_H
#define CHOOSEGENERATORS_H
#include "ui_choosegenerators.h"

#include <Mandel.h>

#include "Bitmap.h"

#include <QDialog>
#include <QValidator>
#include <QLineEdit>
#include <QComboBox>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include <memory>
#include <map>

namespace Ui
{
    class ChooseGenerators;
}


class Benchmarker : public QObject, public QRunnable
{
    Q_OBJECT
private:
    mnd::MandelContext& mndContext;
    mnd::MandelGenerator& generator;
    int row;
    float percentage;
    static const std::vector<mnd::MandelInfo> benches;
public:
    inline Benchmarker(mnd::MandelContext& mndContext, mnd::MandelGenerator& generator, int row, float percentage) :
        mndContext{ mndContext },
        generator{ generator },
        row{ row },
        percentage{ percentage }
    {
    }

    virtual ~Benchmarker(void) override;

    static mnd::MandelViewport benchViewport(void);

    std::pair<long long, std::chrono::nanoseconds> measureMips(const std::function<Bitmap<float>*()>& bench) const;
    double benchmarkResult(mnd::MandelGenerator& mg) const;

    void run(void) override;

signals:
    void finished(int row, float percentage, double mips);
};


class ChooseGenerators : public QDialog
{
    Q_OBJECT
private:
    Ui::ChooseGenerators* sadfgsdfg;
    std::unique_ptr<Ui::ChooseGenerators> ui;
    mnd::MandelContext& mndCtxt;
    std::map<QString, mnd::MandelGenerator*> generators;
    std::vector<std::pair<QLineEdit*, QComboBox*>> tableContent;
    std::unique_ptr<QValidator> floatValidator;
    //std::unique_ptr<mnd::AdaptiveGenerator> createdGenerator;
    std::unique_ptr<mnd::MandelGenerator> chosenGenerator;
    std::vector<mnd::MandelGenerator*> actualGenerators;
    QThreadPool benchmarker;
public:
    ChooseGenerators(mnd::MandelContext& mndCtxt, QWidget* parent = nullptr);
    ~ChooseGenerators();

    inline mnd::MandelGenerator* getChosenGenerator(void) { return chosenGenerator.get(); }

private:
    QComboBox* createComboBox(void);
    QLineEdit* createFloatText(void);
public slots:

private slots:
    void setBenchmarkResult(int row, float percentage, double mips);
    void on_buttonBox_accepted();
    void on_run_clicked();
    void on_generatorTable_cellDoubleClicked(int row, int column);
    void on_compile_clicked();
};

#endif // CHOOSEGENERATORS_H

