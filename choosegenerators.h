#ifndef CHOOSEGENERATORS_H
#define CHOOSEGENERATORS_H
#include "ui_choosegenerators.h"

#include "Mandel.h"

#include <QDialog>
#include <QValidator>
#include <QLineEdit>
#include <QComboBox>
#include <memory>
#include <map>

namespace Ui
{
    class ChooseGenerators;
}


class ChooseGenerators : public QDialog
{
    Q_OBJECT
private:
    std::unique_ptr<Ui::ChooseGenerators> ui;
    mnd::MandelContext& mndCtxt;
    std::map<QString, mnd::Generator*> generators;
    std::vector<std::pair<QLineEdit*, QComboBox*>> tableContent;
    std::unique_ptr<QValidator> floatValidator;
    std::unique_ptr<mnd::AdaptiveGenerator> createdGenerator;
public:
    ChooseGenerators(mnd::MandelContext& mndCtxt, QWidget* parent = nullptr);
    ~ChooseGenerators();

    inline mnd::AdaptiveGenerator* getChosenGenerator(void) { return createdGenerator.get(); }

private:
    QComboBox* createComboBox(void);
    QLineEdit* createFloatText(void);
public slots:

private slots:
    void on_buttonBox_accepted();
};

#endif // CHOOSEGENERATORS_H

