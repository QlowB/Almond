#include "gradientchoosedialog.h"

#include <QtXml/QDomDocument>

GradientChooseDialog::GradientChooseDialog()
{
    gcd.setupUi(this);
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    gcd.plainTextEdit->setFont(f);
}

void GradientChooseDialog::on_buttonBox_accepted()
{
    QDomDocument xsr;
    xsr.setContent(gcd.plainTextEdit->toPlainText());
    auto elem = xsr.documentElement();
    auto colors = xsr.elementsByTagName("color");
    for (int i = 0; i < colors.length(); ++i) {
        auto child = colors.item(i).toElement();

        uint8_t r = child.attributeNode("r").value().toInt();
        uint8_t g = child.attributeNode("g").value().toInt();
        uint8_t b = child.attributeNode("b").value().toInt();

        printf("rgb (%s): %d, %d, %d\n", child.text().toUtf8().data(), r, g, b);
    }
    printf("yee: %s\n", elem.text().toUtf8().data());
    fflush(stdout);
}


void GradientChooseDialog::on_buttonBox_clicked(QAbstractButton *button)
{

}
