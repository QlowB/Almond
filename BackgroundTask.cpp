#include "BackgroundTask.h"

BackgroundTask::BackgroundTask(const std::string& shortDescription, std::function<bool(void)> stopCallback) :
    shortDescription{ shortDescription },
    stopCallback{ std::move(stopCallback) }
{
}


ImageExportTask::ImageExportTask(const alm::ImageExportInfo& iei, std::function<bool(void)> stopCallback) :
    BackgroundTask{ "Exporting Image", std::move(stopCallback) },
    iei{ iei }
{
}


void ImageExportTask::run(void)
{
    try {
        alm::exportImage(iei, [this](float percentage) {
            emit progress(percentage);
        }, stopCallback);
        if (!stopCallback())
            emit finished(true, "Image successfully exported.");
        else
            emit finished(false, "Image export cancelled.");
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
        if (!stopCallback())
            emit finished(true, "Video successfully exported.");
        else
            emit finished(false, "Video export cancelled.");
    }
    catch (alm::VideoExportException& ex) {
        emit finished(false, QString("Error during video export: ") + ex.what());
    }
    catch (...) {
        emit finished(false, "Unknown error occurred during video export.");
    }
}

