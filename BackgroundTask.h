#ifndef BACKGROUNDTASK_H
#define BACKGROUNDTASK_H

#include <QRunnable>

class BackgroundTask
{
    QRunnable* task;
public:
    BackgroundTask(QRunnable* task);

    QRunnable* getRunnable(void);
};

#endif // BACKGROUNDTASK_H
