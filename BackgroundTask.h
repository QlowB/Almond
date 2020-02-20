#ifndef BACKGROUNDTASK_H
#define BACKGROUNDTASK_H

#include <QRunnable>

class BackgroundTask
{
public:
    BackgroundTask(QRunnable* task);
};

#endif // BACKGROUNDTASK_H
