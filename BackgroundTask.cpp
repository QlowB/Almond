#include "BackgroundTask.h"

BackgroundTask::BackgroundTask(const std::string& shortDescription) :
    shortDescription{ shortDescription }
{
}


ImageExportTask::ImageExportTask(const alm::ImageExportInfo& iei) :
    BackgroundTask{ "Exporting Image" },
    iei{ iei }
{
}


void ImageExportTask::run(void)
{
    try {
        alm::exportPng(iei, [this](float percentage) {
            emit progress(percentage);
        });
        emit finished(true, "Image successfully exported.");
    }
    catch (alm::ImageExportException& ex) {
        emit finished(false, QString("Error during image export: ") + ex.what());
    }
    catch (...) {
        emit finished(false, "Unknown error occurred during image export.");
    }
}


VideoExportTask::VideoExportTask(MandelVideoGenerator mvg, mnd::MandelGenerator& generator) :
    BackgroundTask{ "Exporting Video" },
    mvg{ std::move(mvg) },
    generator{ generator }
{
}


void VideoExportTask::run(void)
{
    try {
        mvg.addProgressCallback([this](const MandelVideoProgressInfo& mvpi) {
            emit progress(mvpi.progress);
        });
        mvg.generate(generator);
        emit finished(true, "Video successfully exported.");
    }
    catch (alm::VideoExportException& ex) {
        emit finished(false, QString("Error during video export: ") + ex.what());
    }
    catch (...) {
        emit finished(false, "Unknown error occurred during video export.");
    }
}

