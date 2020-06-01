#include "GradientMenu.h"
#include "ui_GradientMenu.h"

#include <QFile>

const QString GradientMenu::presetNames[] = {
    "blue gold",
    "clouds",
    "default",
    "grayscale",
    "peach",
    "rainbow"
};

GradientMenu::GradientMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GradientMenu)
{
    ui->setupUi(this);
    ui->gradientWidget->setGradient({
        std::vector<std::pair<RGBColor, float>> {
            { RGBColor{ 10, 200, 20 }, 0.1 },
            { RGBColor{ 100, 20, 120 }, 0.7 }
        },
        1.0f
    });

    for (const auto& presetName : presetNames) {
        ui->presetCmb->addItem(presetName);
    }
    connect(ui->gradientWidget, &GradientWidget::gradientChanged, this, &GradientMenu::gradientChanged);
}


GradientMenu::~GradientMenu()
{
    delete ui;
}


const Gradient& GradientMenu::getGradient(void)
{
    return ui->gradientWidget->getGradient();
}


const Gradient& GradientMenu::getGradientBefore(void) const
{
    return before;
}


void GradientMenu::setGradient(Gradient grad)
{
    before = grad;
    ui->gradientWidget->setGradient(std::move(grad));
}

void GradientMenu::on_removeBtn_clicked()
{
    ui->gradientWidget->removeSelectedHandle();
}

void GradientMenu::on_presetCmb_currentIndexChanged(int index)
{
    QString presetName = presetNames[index];
    QFile gradXml{ ":/gradients/" + presetName };
    if (gradXml.open(QFile::ReadOnly)) {
        QString xml = QString::fromUtf8(gradXml.readAll());
        ui->gradientWidget->setGradient(Gradient::fromXml(xml.toStdString()));
        gradXml.close();
    }
}
