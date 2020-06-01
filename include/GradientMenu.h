#ifndef GRADIENTMENU_H
#define GRADIENTMENU_H

#include <QWidget>
#include <QVector>
#include <QPair>

#include "Gradient.h"

namespace Ui {
class GradientMenu;
}

class GradientMenu : public QWidget
{
    Q_OBJECT

    Ui::GradientMenu *ui;
    Gradient before;

    static const QString presetNames[];
public:
    explicit GradientMenu(QWidget *parent = nullptr);
    ~GradientMenu(void);

    const Gradient& getGradient(void);
    const Gradient& getGradientBefore(void) const;
    void setGradient(Gradient grad);

signals:
    void gradientChanged(void);
private slots:
    void on_removeBtn_clicked();
    void on_presetCmb_currentIndexChanged(int index);
};

#endif // GRADIENTMENU_H
