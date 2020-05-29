#ifndef BACKGROUNDTASK_H
#define BACKGROUNDTASK_H

#include <QObject>
#include <QRunnable>
#include <string>
#include <functional>
#include "ImageExport.h"
#include "MandelVideoGenerator.h"

class BackgroundTask : public QObject, public QRunnable
{
    Q_OBJECT
protected:
    std::string shortDescription;
    std::function<bool(void)> stopCallback;
public:
    BackgroundTask(const std::string& shortDescription, std::function<bool(void)> stopCallback = [] () { return false; });

    void run(void) = 0;

    inline const std::string& getShortDescription(void) const { return shortDescription; }

signals:
    void progress(float percentage);
    void finished(bool success, QString message);
};


class ImageExportTask : public BackgroundTask
{
    Q_OBJECT
private:
    const alm::ImageExportInfo iei;
public:
    ImageExportTask(const alm::ImageExportInfo& iei, std::function<bool(void)> stopCallback = [] () { return false; });

    void run(void) override;
};


class VideoExportTask : public BackgroundTask
{
    Q_OBJECT
private:
    MandelVideoGenerator mvg;
    mnd::MandelGenerator& generator;
public:
    VideoExportTask(MandelVideoGenerator mvg, mnd::MandelGenerator& generator);

    void run(void) override;
};

#endif // BACKGROUNDTASK_H
