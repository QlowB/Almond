#ifndef GRADIENTMENU_H
#define GRADIENTMENU_H

#include <QWidget>
#include <QVector>
#include <QPair>
#include <QFile>

#include "Gradient.h"

namespace Ui {
class GradientMenu;
}


class GradientMenu : public QWidget
{
    Q_OBJECT

    Ui::GradientMenu *ui;
    alm::Gradient before;

    static const QString presetNames[];
public:
    explicit GradientMenu(QWidget *parent = nullptr);
    ~GradientMenu(void);

    const alm::Gradient& getGradient(void);
    const alm::Gradient& getGradientBefore(void) const;
    void setGradient(alm::Gradient grad);

    void loadGradient(QFile& file);

signals:
    void gradientChanged(void);
private slots:
    void onGradientChanged(void);
    void on_removeBtn_clicked();
    void on_presetCmb_currentIndexChanged(int index);
    void on_saveBtn_clicked();
    void on_loadBtn_clicked();
    void on_maxValSpb_valueChanged(double maxVal);
    void on_zoomOutBtn_clicked();
    void on_zoomInBtn_clicked();
    void on_zoomResetBtn_clicked();
};

#endif // GRADIENTMENU_H
