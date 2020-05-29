#ifndef GRADIENTCHOOSEDIALOG_H
#define GRADIENTCHOOSEDIALOG_H

#include <QDialog>
#include "ui_gradientchooser.h"
#include "Gradient.h"

#include <memory>
#include <map>

class GradientChooseDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::GradientChooser gcd;
    std::unique_ptr<Gradient> chosenGradient = nullptr;
public:
    GradientChooseDialog();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_presets_currentIndexChanged(const QString &arg1);

public:
    inline std::unique_ptr<Gradient> getGradient(void) { return std::move(chosenGradient); }
};

#endif // GRADIENTCHOOSEDIALOG_H
