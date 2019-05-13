#pragma once
#ifndef SECTIONMANAGER_H
#define SECTIONMANAGER_H

#include "QueueManager.h"

class SectionManager
{
public:
    SectionManager(void);

    int getNextLevel(double chunkWidth);
};

#endif // SECTIONMANAGER_H
