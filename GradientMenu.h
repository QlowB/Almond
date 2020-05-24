#ifndef GRADIENTMENU_H
#define GRADIENTMENU_H

#include <QWidget>
#include <QVector>
#include <QPair>

namespace Ui {
class GradientMenu;
}

class GradientMenu : public QWidget
{
    Q_OBJECT

    Ui::GradientMenu *ui;
public:
    explicit GradientMenu(QWidget *parent = nullptr);
    ~GradientMenu(void);

    const QVector<QPair<float, QColor>>& getGradient(void);
    void setGradient(QVector<QPair<float, QColor>> grad);

signals:
    void gradientChanged(void);
};

#endif // GRADIENTMENU_H
