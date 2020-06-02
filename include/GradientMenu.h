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


class MinHeightWrapperWidget :
    public QWidget
{
    Q_OBJECT
    int minHeight;
    QWidget* widget;
public:
    MinHeightWrapperWidget(QWidget* contains, QWidget* parent);
    QSize minimumSizeHint(void) const override;
    QSize sizeHint(void) const override;
};


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
