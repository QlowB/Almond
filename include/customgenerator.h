#ifndef CUSTOMGENERATOR_H
#define CUSTOMGENERATOR_H

#include <QDialog>

#include <Mandel.h>
#include <IterationCompiler.h>


struct FractalDef
{
    QString name;
    QString z0;
    QString zi;
    mnd::GeneratorCollection gc;
};


namespace Ui {
class CustomGenerator;
}

class CustomGenerator : public QDialog
{
    Q_OBJECT

    mnd::MandelContext& mndCtxt;
    std::vector<FractalDef> fractalDefs;
public:
    explicit CustomGenerator(mnd::MandelContext& mndCtxt, QWidget *parent = nullptr);
    ~CustomGenerator();

    FractalDef* getLastCompiled(void);

    void compile();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::CustomGenerator *ui;
};

#endif // CUSTOMGENERATOR_H
