#include "BackgroundTask.h"
#include <QMessageBox>

BackgroundTask::BackgroundTask(const std::string& shortDescription) :
    shortDescription{ shortDescription }
{
}


ImageExportTask::ImageExportTask(const alm::ImageExportInfo& iei) :
    BackgroundTask{ "exporting image" },
    iei{ iei }
{
}


void ImageExportTask::run(void)
{
    alm::exportPng(iei, [this](float percentage) {
        emit progress(percentage);
    });
    emit finished(true);
}


VideoExportTask::VideoExportTask(MandelVideoGenerator mvg, mnd::MandelGenerator& generator) :
    BackgroundTask{ "exporting video" },
    mvg{ std::move(mvg) },
    generator{ generator }
{
}


void VideoExportTask::run(void)
{

    mvg.addProgressCallback([this](const MandelVideoProgressInfo& mvpi) {
        emit progress(0);
    });
    mvg.generate(generator);
    emit finished(true);
    QMessageBox* msgBox = new QMessageBox;
    msgBox->setText("Video successfully exported.");
    msgBox->exec();
}

