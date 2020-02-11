#include "gradientchoosedialog.h"

#include <QtXml/QDomDocument>
#include <QFile>
#include <QResource>
#include <QDir>


//resource hacks
static const std::string clouds_xml = "";

std::map<std::string, std::string> GradientChooseDialog::presets {
    { "clouds", clouds_xml }
};


GradientChooseDialog::GradientChooseDialog()
{
    gcd.setupUi(this);
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    gcd.plainTextEdit->setFont(f);


    QFile qf(":/Almond/clouds");
    QString str = QString::fromUtf8(qf.readAll());
    printf("%s\n", str.toStdString().c_str());
    gcd.presets->addItem("clouds");
    gcd.presets->addItem("none");
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

void GradientChooseDialog::on_presets_currentIndexChanged(const QString& index)
{
    QResource gr(":/gradients/clouds.xml");
    QString str = QString::fromUtf8(reinterpret_cast<const char*>(gr.data()));
    emit gcd.plainTextEdit->setPlainText(str);
}
