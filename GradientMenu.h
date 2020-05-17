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

public:
    explicit GradientMenu(QWidget *parent = nullptr);
    ~GradientMenu(void);

    const QVector<QPair<float, QColor>>& getGradient(void);

private:
    Ui::GradientMenu *ui;
};

#endif // GRADIENTMENU_H
