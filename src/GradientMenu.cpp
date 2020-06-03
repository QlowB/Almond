#include "GradientMenu.h"
#include "ui_GradientMenu.h"

#include "Serialize.h"
#include "XmlException.h"

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextEdit>

using alm::Gradient;

const QString GradientMenu::presetNames[] = {
    "blue gold",
    "clouds",
    "oldschool",
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
    connect(ui->gradientWidget, &GradientWidget::gradientChanged, this, &GradientMenu::onGradientChanged);
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


void GradientMenu::loadGradient(QFile& file)
{
    if (file.isOpen() || file.open(QFile::ReadOnly)) {
        QString xml = QString::fromUtf8(file.readAll());
        try {
            ui->gradientWidget->setGradient(alm::loadGradient(xml.toStdString()));
        } catch (alm::XmlException& xmlex) {
            QMessageBox::critical(this, tr("Error Loading Gradient"), tr("Error loading gradient: ") + xmlex.what());
        } catch (...) {
            QMessageBox::critical(this, tr("Error Loading Gradient"), tr("Unknown error loading gradient."));
        }
    }
}


void GradientMenu::onGradientChanged(void)
{
    ui->maxValSpb->setValue(ui->gradientWidget->getGradient().getMax());
}


void GradientMenu::on_removeBtn_clicked()
{
    ui->gradientWidget->removeSelectedHandle();
}

void GradientMenu::on_presetCmb_currentIndexChanged(int index)
{
    QString presetName = presetNames[index];
    QFile gradXml{ ":/gradients/" + presetName };
    loadGradient(gradXml);
}

void GradientMenu::on_saveBtn_clicked()
{
    std::string xml = alm::saveGradient(ui->gradientWidget->getGradient());
    QString filename =
            QFileDialog::getSaveFileName(this, tr("Save Gradient"), "", "Gradient XML Files (*.xml)");
    if (!filename.isNull()) {
        QFile saveFile{ filename };
        bool opened = saveFile.open(QFile::WriteOnly);
        if (!opened) {
            QMessageBox::critical(this, tr("Error saving file"), tr("Error saving gradient: Could not open file."));
        }
        saveFile.write(QString::fromStdString(xml).toUtf8());
        saveFile.close();
    }
}

void GradientMenu::on_loadBtn_clicked()
{
    QFileDialog openDialog{ this, tr("Load Gradient"), "", "" };
    connect(&openDialog, &QFileDialog::fileSelected, [this] (const QString& name) {
        QFile file{ name };
        loadGradient(file);
    });
    openDialog.exec();
}


void GradientMenu::on_maxValSpb_valueChanged(double maxVal)
{
    const Gradient& old = ui->gradientWidget->getGradient();
    float minVal = old.getPoints().at(old.getPoints().size() - 1).second;
    if (maxVal < minVal) {
        ui->maxValSpb->setValue(minVal);
        maxVal = minVal;
    }
    Gradient g = Gradient{ old.getPoints(), float(maxVal), old.isRepeat() };
    ui->gradientWidget->setGradient(std::move(g));
}


void GradientMenu::on_zoomOutBtn_clicked()
{
    int currentHeight = ui->gradientWidget->height();
    int minHeight = ui->gradientWidget->sizeHint().height();
    if (currentHeight * 0.9 >= minHeight) {
        ui->gradientWidget->setFixedHeight(int(currentHeight * 0.9));
    }
    else {
        ui->gradientWidget->setFixedHeight(minHeight);
    }
}


void GradientMenu::on_zoomInBtn_clicked()
{
    int currentHeight = ui->gradientWidget->height();
    int maxHeight = 16000;
    if (currentHeight * 1.1 <= maxHeight)
        ui->gradientWidget->setFixedHeight(currentHeight * 1.1);
    else
        ui->gradientWidget->setFixedHeight(maxHeight);
}


void GradientMenu::on_zoomResetBtn_clicked()
{
    ui->gradientWidget->setMinimumHeight(ui->gradientWidget->minimumSizeHint().height());
    ui->gradientWidget->setMaximumHeight(16000);
}
