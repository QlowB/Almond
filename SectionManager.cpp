#include "SectionManager.h"

SectionManager::SectionManager(void)
{

}


int SectionManager::getNextLevel(double chunkWidth)
{
    int level = 0;
    double w = 1;
    while(w >= chunkWidth) {
        w = w / 2;
        level++;
    }
    return level;
}

