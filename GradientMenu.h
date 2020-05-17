#ifndef GRADIENTMENU_H
#define GRADIENTMENU_H

#include <QWidget>

namespace Ui {
class GradientMenu;
}

class GradientMenu : public QWidget
{
    Q_OBJECT

public:
    explicit GradientMenu(QWidget *parent = nullptr);
    ~GradientMenu();

private:
    Ui::GradientMenu *ui;
};

#endif // GRADIENTMENU_H
