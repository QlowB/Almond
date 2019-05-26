#include "exportdialogs.h"

#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>

#include "Mandel.h"
#include "VideoStream.h"

static bool exportVideo(const ExportVideoInfo& evi);

ExportImageDialog::ExportImageDialog(QWidget* parent) :
    QDialog{ parent }
{
    eid.setupUi(this);
    eid.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
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
        msgBox.show();
        msgBox.exec();
        reject();
    }
}

ExportVideoDialog::ExportVideoDialog(QWidget* parent, const ExportVideoInfo& evi) :
    QDialog{ parent },
    evi{ evi }
{
    evd.setupUi(this);
    evd.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
    evd.vidWidth->setValidator(new QIntValidator(1, 10000000, this));
    evd.vidHeight->setValidator(new QIntValidator(1, 10000000, this));

    evd.startX->setText(QString::number(evi.start.x));
    evd.startY->setText(QString::number(evi.start.y));
    evd.startW->setText(QString::number(evi.start.width));
    evd.startH->setText(QString::number(evi.start.height));

    evd.endX->setText(QString::number(evi.end.x));
    evd.endY->setText(QString::number(evi.end.y));
    evd.endW->setText(QString::number(evi.end.width));
    evd.endH->setText(QString::number(evi.end.height));
}

const ExportVideoInfo& ExportVideoDialog::getExportVideoInfo(void) const
{
    return evi;
}

void ExportVideoDialog::on_buttonBox_accepted()
{
    if (evd.savePath->text() == "") {
        QMessageBox* msgBox = new QMessageBox;
        msgBox->setText("Please specify a path.");
        msgBox->exec();
        emit reject();
    }

    evi.path = evd.savePath->text();
    evi.width = evd.vidWidth->text().toInt();
    evi.height = evd.vidHeight->text().toInt();
    evi.maxIterations = evd.maxIterations->text().toInt();
    /*evi.start = mnd::MandelViewport {
        evd.startX->text().toDouble(),
        evd.startY->text().toDouble(),
        evd.startW->text().toDouble(),
        evd.startH->text().toDouble(),
    };
    evi.end = mnd::MandelViewport {
        evd.endX->text().toDouble(),
        evd.endY->text().toDouble(),
        evd.endW->text().toDouble(),
        evd.endH->text().toDouble(),
    };*/

    evi.start.adjustAspectRatio(evi.width, evi.height);
    evi.end.adjustAspectRatio(evi.width, evi.height);

    if (exportVideo(evi)) {
        QMessageBox* msgBox = new QMessageBox;
        msgBox->setText("Video successfully exported.");
        msgBox->exec();
    }
}

void ExportVideoDialog::on_pushButton_clicked()
{
    QString saveAs = QFileDialog::getSaveFileName(this,
            tr("Save exported image"), "",
            tr("MPEG video (*.mp4);;All Files (*)"));
    evd.savePath->setText(saveAs);
    this->repaint();
}


bool exportVideo(const ExportVideoInfo& evi)
{
    auto lerp = [] (double a, double b, double v) {
        return a * (1 - v) + b * v;
    };

    mnd::MandelContext ctxt = mnd::initializeContext();
    mnd::Generator& gen = *ctxt.getDevices()[0].getGeneratorDouble();
    mnd::MandelInfo mi;
    mi.bWidth = evi.width;
    mi.bHeight = evi.height;
    mi.maxIter = evi.maxIterations;

    VideoStream vs(evi.width, evi.height, evi.path.toStdString());

    double x = evi.end.x + evi.end.width / 2;
    double y = evi.end.y + evi.end.height / 2;
    double w = evi.start.width;
    double h = evi.start.height;

    while(w > evi.end.width || h > evi.end.height) {
        mi.view = mnd::MandelViewport{ x - w/2, y - h/2, w, h };

        Bitmap<float> raw{ evi.width, evi.height };
        gen.generate(mi, raw.pixels.get());
        vs.addFrame(raw.map<RGBColor>([] (float x) { return
            RGBColor{ uint8_t(::sin(x / 100) * 127 + 127), uint8_t(::sin(x / 213) * 127 + 127), uint8_t(::cos(x / 173) * 127 + 127) };
        }));

        w *= 0.975;
        h *= 0.975;
    }
    return true;
}