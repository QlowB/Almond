#include "gradientchoosedialog.h"

#include <QtXml/QDomDocument>

GradientChooseDialog::GradientChooseDialog()
{
    gcd.setupUi(this);
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    gcd.plainTextEdit->setFont(f);
}

#if 0

<gradient>
  <color r="230" g="10" b="40" p="0" />
  <color r="30" g="230" b="80" p="100" />
  <color r="20" g="120" b="210" p="500" />
</gradient>

#endif

void GradientChooseDialog::on_buttonBox_accepted()
{
    QDomDocument xsr;
    xsr.setContent(gcd.plainTextEdit->toPlainText());
    auto elem = xsr.documentElement();
    auto colors = xsr.elementsByTagName("color");
    std::vector<std::pair<RGBColor, float>> colorArr;
    for (int i = 0; i < colors.length(); ++i) {
        auto child = colors.item(i).toElement();
        uint8_t r = child.attributeNode("r").value().toInt();
        uint8_t g = child.attributeNode("g").value().toInt();
        uint8_t b = child.attributeNode("b").value().toInt();
        float p = child.attributeNode("p").value().toInt();

        printf("rgb (%s): %d, %d, %d\n", child.text().toUtf8().data(), r, g, b);
        colorArr.push_back({ { r, g, b }, p });
    }

    chosenGradient = std::make_unique<Gradient>(colorArr);

    printf("yee: %s\n", elem.text().toUtf8().data());
    fflush(stdout);
}


void GradientChooseDialog::on_buttonBox_clicked(QAbstractButton *button)
{

}
