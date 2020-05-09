#include "gradientchoosedialog.h"

#include <QFile>
#include <QResource>

#include <QColorDialog>

GradientChooseDialog::GradientChooseDialog()
{
    gcd.setupUi(this);
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    gcd.plainTextEdit->setFont(f);

    gcd.presets->addItem("default");
    gcd.presets->addItem("clouds");
    gcd.presets->addItem("rainbow");
    gcd.presets->addItem("grayscale");
    gcd.presets->addItem("peach");
    gcd.presets->addItem("blue gold");
}


void GradientChooseDialog::on_buttonBox_accepted()
{
    chosenGradient = std::make_unique<Gradient>(Gradient::readXml(gcd.plainTextEdit->toPlainText()));
}


void GradientChooseDialog::on_buttonBox_clicked(QAbstractButton*)
{
}


void GradientChooseDialog::on_presets_currentIndexChanged(const QString& index)
{
    QFile res(":/gradients/" + index);
    res.open(QIODevice::ReadOnly);
    QString str = QString::fromUtf8(res.readAll());
    emit gcd.plainTextEdit->setPlainText(str);
}
