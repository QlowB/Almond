#include "Almond.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>
#include "benchmarkdialog.h"

Almond::Almond(QWidget *parent) :
    QMainWindow(parent),
    mandelContext(mnd::initializeContext())
{
    ui.setupUi(this);
    printf("not yet created!\n");
    mw = std::make_unique<MandelWidget>(mandelContext, ui.centralWidget);
    //qRegisterMetaType<MandelWidget>("MandelWidget");
    printf("created!\n");
    ui.verticalLayout_left->addWidget(mw.get());
    //ui.verticalLayout_left->addWidget(new MyGLWidget(ui.centralWidget));
    //mw->show();
}

void Almond::on_pushButton_clicked()
{
    ExportImageDialog dialog(this);
    auto response = dialog.exec();
    if (response == 1) {
        mnd::MandelInfo mi;
        mi.maxIter = dialog.getMaxIterations();
        mi.view = mw->getViewport();
        mi.bWidth = dialog.getWidth();
        mi.bHeight = dialog.getHeight();
        mi.view.adjustAspectRatio(mi.bWidth, mi.bHeight);
        mnd::Generator& g = mandelContext.getDefaultGenerator();
        auto fmap = Bitmap<float>(mi.bWidth, mi.bHeight);
        g.generate(mi, fmap.pixels.get());
        auto bitmap = fmap.map<RGBColor>([](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
        QImage img((unsigned char*)bitmap.pixels.get(), bitmap.width, bitmap.height, bitmap.width * 3, QImage::Format_RGB888);
        img.save(dialog.getPath());
    }
}


ExportImageDialog::ExportImageDialog(QWidget* parent) :
    QDialog{ parent }
{
    eid.setupUi(this);
    eid.maxIterations->setValidator(new QIntValidator(1, 10000000, this));
    eid.imgWidth->setValidator(new QIntValidator(1, 10000000, this));
    eid.imgHeight->setValidator(new QIntValidator(1, 10000000, this));
}


int ExportImageDialog::getMaxIterations(void) const
{
    return std::stoi(eid.maxIterations->text().toStdString());
}


int ExportImageDialog::getWidth(void) const
{
    return std::stoi(eid.imgWidth->text().toStdString());
}


int ExportImageDialog::getHeight(void) const
{
    return std::stoi(eid.imgHeight->text().toStdString());
}


QString ExportImageDialog::getPath(void) const
{
    return eid.savePath->text();
}

void ExportImageDialog::on_pushButton_clicked()
{
    QString saveAs = QFileDialog::getSaveFileName(this,
            tr("Save exported image"), "",
            tr("PNG image (*.png);;JPEG image (*.jpg);;All Files (*)"));
    eid.savePath->setText(saveAs);
    this->repaint();
}

void ExportImageDialog::on_buttonBox_accepted()
{
    if (eid.savePath->text() == "") {
        QMessageBox msgBox;
        msgBox.setText("Please specify a path.");
        msgBox.exec();
        reject();
    }
}

void Almond::on_pushButton_2_clicked()
{
    BenchmarkDialog bd(mandelContext, this);
    bd.exec();
}
