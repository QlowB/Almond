#include "ExportImageMenu.h"
#include "ui_ExportImageMenu.h"

#include "ImageExport.h"

#include <QIntValidator>
#include <QFileDialog>

ExportImageMenu::ExportImageMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportImageMenu)
{
    ui->setupUi(this);
    ui->maxIterTxt->setValidator(new QIntValidator(1, 1000000000, this));
    ui->widthTxt->setValidator(new QIntValidator(1, 10000000, this));
    ui->heightTxt->setValidator(new QIntValidator(1, 10000000, this));
}


ExportImageMenu::~ExportImageMenu()
{
    delete ui;
}


int ExportImageMenu::getMaxIterations(void) const
{
    return ui->maxIterTxt->text().toInt();
}


int ExportImageMenu::getWidth(void) const
{
    return ui->widthTxt->text().toInt();
}


int ExportImageMenu::getHeight(void) const
{
    return ui->heightTxt->text().toInt();
}


QString ExportImageMenu::getPath(void) const
{
    return ui->pathTxt->text();
}


void ExportImageMenu::on_pathBtn_clicked()
{
    std::string formatString = "";
    if (alm::supportsImageFormat(alm::ImageFormat::PNG)) {
        formatString += "PNG image (*.png);;";
    }
    if (alm::supportsImageFormat(alm::ImageFormat::JPEG)) {
        formatString += "JPEG image (*.jpg *.jpeg);;";
    }
    if (alm::supportsImageFormat(alm::ImageFormat::BMP)) {
        formatString += "BMP image (*.bmp *.dib);;";
    }
    if (!formatString.empty()) {
        formatString.erase(formatString.end() - 2, formatString.end());
    }

    QString saveAs = QFileDialog::getSaveFileName(this,
            tr("Save exported image"), "",
            formatString.c_str());
    if(!saveAs.isEmpty() && !saveAs.isNull())
        ui->pathTxt->setText(saveAs);
}
