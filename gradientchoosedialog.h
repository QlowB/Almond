#ifndef GRADIENTCHOOSEDIALOG_H
#define GRADIENTCHOOSEDIALOG_H

#include <QDialog>
#include "ui_gradientchooser.h"

class GradientChooseDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::GradientChooser gcd;
public:
    GradientChooseDialog();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_clicked(QAbstractButton *button);
};

#endif // GRADIENTCHOOSEDIALOG_H
