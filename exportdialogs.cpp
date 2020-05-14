#include "exportdialogs.h"

#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>

#include "Mandel.h"
#include "Almond.h"
#include "VideoStream.h"

//static bool exportVideo(const ExportVideoInfo& evi);

ExportImageDialog::ExportImageDialog(QWidget* parent) :
    QDialog{ parent }
{
    eid.setupUi(this);
    this->adjustSize();
    eid.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
    eid.imgWidth->setValidator(new QIntValidator(1, 10000000, this));
    eid.imgHeight->setValidator(new QIntValidator(1, 10000000, this));
}


void ExportImageDialog::setMaxIterations(int mi)
{
    eid.maxIterations->setText(QString::number(mi));
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
            tr(formatString.c_str()));
    if(!saveAs.isEmpty() && !saveAs.isNull())
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


ExportVideoDialog::ExportVideoDialog(Almond *parent, const ExportVideoInfo& evi) :
    QDialog{ parent },
    almond{ parent },
    evi{ evi }
{
    evd.setupUi(this);
    this->adjustSize();
    evd.maxIterations->setValidator(new QIntValidator(1, 1000000000, this));
    evd.vidWidth->setValidator(new QIntValidator(1, 10000000, this));
    evd.vidHeight->setValidator(new QIntValidator(1, 10000000, this));
    evd.bitrate->setValidator(new QIntValidator(1, 10000000, this));
    evd.fps->setValidator(new QIntValidator(1, 8000, this));
    evd.zoomSpeed->setValidator(new QDoubleValidator(0.0, 100.0, -1, this));

#ifdef WITH_BOOST
    evd.startX->setText(QString::fromStdString(evi.start.x.str()));
    evd.startY->setText(QString::fromStdString(evi.start.y.str()));
    evd.startW->setText(QString::fromStdString(evi.start.width.str()));
    evd.startH->setText(QString::fromStdString(evi.start.height.str()));

    evd.endX->setText(QString::fromStdString(evi.end.x.str()));
    evd.endY->setText(QString::fromStdString(evi.end.y.str()));
    evd.endW->setText(QString::fromStdString(evi.end.width.str()));
    evd.endH->setText(QString::fromStdString(evi.end.height.str()));
#else
    evd.startX->setText(QString::number(evi.start.x));
    evd.startY->setText(QString::number(evi.start.y));
    evd.startW->setText(QString::number(evi.start.width));
    evd.startH->setText(QString::number(evi.start.height));

    evd.endX->setText(QString::number(evi.end.x));
    evd.endY->setText(QString::number(evi.end.y));
    evd.endW->setText(QString::number(evi.end.width));
    evd.endH->setText(QString::number(evi.end.height));
#endif // WITH_BOOST

    auto presets = {
        "ultrafast",
        "superfast",
        "veryfast",
        "faster",
        "fast",
        "medium",
        "slow",
        "slower",
        "veryslow",
    };
    for (auto& preset : presets) {
        evd.encodingPresetBox->addItem(preset);
    }
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

    evi.path = evd.savePath->text().toStdString();

    evi.mi = almond->mw->getMandelInfo();
    evi.mi.bWidth = evd.vidWidth->text().toInt();
    evi.mi.bHeight = evd.vidHeight->text().toInt();
    evi.mi.maxIter = evd.maxIterations->text().toInt();

    evi.bitrate = evd.bitrate->text().toInt();
    evi.preset = evd.encodingPresetBox->currentText().toStdString();
    evi.fps = evd.fps->text().toInt();
    evi.zoomSpeed = QLocale::system().toDouble(evd.zoomSpeed->text());
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

    evi.start.adjustAspectRatio(evi.mi.bWidth, evi.mi.bHeight);
    evi.end.adjustAspectRatio(evi.mi.bWidth, evi.mi.bHeight);
    evi.gradient = almond->mw->getGradient();

    //}
}

void ExportVideoDialog::on_pushButton_clicked()
{
    QString saveAs = QFileDialog::getSaveFileName(this,
            tr("Save exported image"), "",
            tr("AVI video (*.avi);;MP4 video (*.mp4);;All Files (*)"));
    evd.savePath->setText(saveAs);
    this->repaint();
}

/*
bool exportVideo(const ExportVideoInfo& evi)
{
    auto lerp = [] (double a, double b, double v) {
        return a * (1 - v) + b * v;
    };

    mnd::MandelContext ctxt = mnd::initializeContext();
    mnd::Generator& gen = *ctxt.getDevices()[0].getGeneratorFloat();
    mnd::MandelInfo mi;
    mi.bWidth = evi.width;
    mi.bHeight = evi.height;
    mi.maxIter = evi.maxIterations;

    VideoStream vs(evi.width, evi.height, evi.path, evi.bitrate, evi.preset.c_str());

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
*/
