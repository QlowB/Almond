#include "ExportVideoMenu.h"
#include "ui_ExportVideoMenu.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QFileDialog>

ExportVideoMenu::ExportVideoMenu(QWidget *parent) :
    QWidget{ parent },
    ui{ new Ui::ExportVideoMenu }
{
    ui->setupUi(this);
    this->adjustSize();
    ui->maxIterTxt->setValidator(new QIntValidator(1, 1000000000, this));
    ui->widthTxt->setValidator(new QIntValidator(1, 10000000, this));
    ui->heightTxt->setValidator(new QIntValidator(1, 10000000, this));
    ui->bitrate->setValidator(new QIntValidator(1, 10000000, this));
    ui->fps->setValidator(new QIntValidator(1, 8000, this));
    ui->zoomSpeed->setValidator(new QDoubleValidator(0.0, 100.0, -1, this));


    const auto presets = {
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
        ui->encodingPresetBox->addItem(preset);
    }
    ui->encodingPresetBox->setCurrentText("medium");
    setStartViewport(mnd::MandelViewport::standardView());
}


ExportVideoMenu::~ExportVideoMenu()
{
    delete ui;
}


ExportVideoInfo ExportVideoMenu::getInfo(void) const
{
    ExportVideoInfo evi;

    evi.path = ui->pathTxt->text().toStdString();

    evi.mi.bWidth = ui->widthTxt->text().toInt();
    evi.mi.bHeight = ui->heightTxt->text().toInt();
    evi.mi.maxIter = ui->maxIterTxt->text().toInt();

    evi.bitrate = ui->bitrate->text().toInt();
    evi.preset = ui->encodingPresetBox->currentText().toStdString();
    evi.fps = ui->fps->text().toInt();
    evi.zoomSpeed = QLocale::system().toDouble(ui->zoomSpeed->text());
    evi.start = mnd::MandelViewport {
        mnd::Real(ui->startX->text().toStdString().c_str()),
        mnd::Real(ui->startY->text().toStdString().c_str()),
        mnd::Real(ui->startW->text().toStdString().c_str()),
        mnd::Real(ui->startH->text().toStdString().c_str())
    };
    evi.end = mnd::MandelViewport {
        mnd::Real(ui->endX->text().toStdString().c_str()),
        mnd::Real(ui->endY->text().toStdString().c_str()),
        mnd::Real(ui->endW->text().toStdString().c_str()),
        mnd::Real(ui->endH->text().toStdString().c_str())
    };

    evi.start.adjustAspectRatio(evi.mi.bWidth, evi.mi.bHeight);
    evi.end.adjustAspectRatio(evi.mi.bWidth, evi.mi.bHeight);

    return evi;
}


void ExportVideoMenu::setStartViewport(const mnd::MandelViewport& mv)
{
    ui->startX->setText(QString::fromStdString(mnd::toString(mv.x)));
    ui->startY->setText(QString::fromStdString(mnd::toString(mv.y)));
    ui->startW->setText(QString::fromStdString(mnd::toString(mv.width)));
    ui->startH->setText(QString::fromStdString(mnd::toString(mv.height)));
}


void ExportVideoMenu::setEndViewport(const mnd::MandelViewport& mv)
{
    ui->endX->setText(QString::fromStdString(mnd::toString(mv.x)));
    ui->endY->setText(QString::fromStdString(mnd::toString(mv.y)));
    ui->endW->setText(QString::fromStdString(mnd::toString(mv.width)));
    ui->endH->setText(QString::fromStdString(mnd::toString(mv.height)));
}


void ExportVideoMenu::on_pathBtn_clicked()
{
    QString saveAs = QFileDialog::getSaveFileName(this,
            tr("Save exported video"), "",
            tr("AVI video (*.avi);;MP4 video (*.mp4);;All Files (*)"));
    ui->pathTxt->setText(saveAs);
}
