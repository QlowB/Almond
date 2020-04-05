#include "BackgroundTask.h"

BackgroundTask::BackgroundTask(QRunnable* q) :
    task{ q }
{
}


QRunnable* BackgroundTask::getRunnable(void)
{
    return nullptr;
}


