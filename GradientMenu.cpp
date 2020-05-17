#include "GradientMenu.h"
#include "ui_GradientMenu.h"

GradientMenu::GradientMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GradientMenu)
{
    ui->setupUi(this);
    ui->gradientWidget->setGradient(
    {
                    {0.1, QColor{10, 200, 20}},
                    {0.7, QColor{100, 20, 120}}
    }
    );
}

GradientMenu::~GradientMenu()
{
    delete ui;
}
