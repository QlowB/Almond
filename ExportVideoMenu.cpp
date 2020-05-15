#include "ExportVideoMenu.h"
#include "ui_ExportVideoMenu.h"
#include <QIntValidator>
#include <QDoubleValidator>

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


    /*
    evd.startX->setText(QString::fromStdString(evi.start.x.str()));
    evd.startY->setText(QString::fromStdString(evi.start.y.str()));
    evd.startW->setText(QString::fromStdString(evi.start.width.str()));
    evd.startH->setText(QString::fromStdString(evi.start.height.str()));

    evd.endX->setText(QString::fromStdString(evi.end.x.str()));
    evd.endY->setText(QString::fromStdString(evi.end.y.str()));
    evd.endW->setText(QString::fromStdString(evi.end.width.str()));
    evd.endH->setText(QString::fromStdString(evi.end.height.str()));
    */


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
    /*if (ui->pathTxt->text() == "") {
        QMessageBox* msgBox = new QMessageBox;
        msgBox->setText("Please specify a path.");
        msgBox->exec();
        emit reject();
    }*/

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

