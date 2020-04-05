#ifndef CHOOSEGENERATORS_H
#define CHOOSEGENERATORS_H
#include "ui_choosegenerators.h"

#include <Mandel.h>
#include <IterationCompiler.h>

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

class Almond;

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
    Almond& owner;
    Ui::ChooseGenerators* sadfgsdfg;
    std::unique_ptr<Ui::ChooseGenerators> ui;
    mnd::MandelContext& mndCtxt;
    std::map<QString, mnd::MandelGenerator*> generators;
    std::vector<std::pair<QLineEdit*, QComboBox*>> tableContent;

    std::unique_ptr<mnd::AdaptiveGenerator> chosenGenerator;

    std::vector<mnd::MandelGenerator*> actualGenerators;

    mnd::AdaptiveGenerator& generator;
    std::vector<mnd::MandelGenerator*> allGenerators;

    std::unique_ptr<QValidator> floatValidator;
    QThreadPool benchmarker;
public:
    /*!
     * \brief create a dialog that lets choose from the default mandelbrot generators
     */
    ChooseGenerators(mnd::MandelContext& mndCtxt, mnd::AdaptiveGenerator& generator, Almond& owner);

    /*!
     * \brief create a dialog that lets choose from the default mandelbrot generators
     */
    ChooseGenerators(mnd::MandelContext& mndCtxt, mnd::GeneratorCollection& gc,
                     mnd::AdaptiveGenerator& generator, Almond& owner);
    ~ChooseGenerators();

    inline std::unique_ptr<mnd::AdaptiveGenerator> extractChosenGenerator(void) { return std::move(chosenGenerator); }

private:

    void initializeTables(void);

    QComboBox* createComboBox(void);
    QLineEdit* createFloatText(void);

    void addRowAt(int index);
public slots:

private slots:
    void setBenchmarkResult(int row, float percentage, double mips);
    void on_buttonBox_accepted();
    void on_run_clicked();
    void on_generatorTable_cellDoubleClicked(int row, int column);
    void on_addRow_clicked();
    void on_removeRow_clicked();
};

#endif // CHOOSEGENERATORS_H

