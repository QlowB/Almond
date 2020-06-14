#include "Util.h"

#include <map>
#include <iostream>
#include <iomanip>


void printTable(const Table& t)
{
    std::map<size_t, size_t> maxLenghts;

    for (const auto& arr : t) {
        for (int i = 0; i < arr.size(); i++) {
            if (arr[i].size() > maxLenghts[i])
                maxLenghts[i] = arr[i].size();
        }
    }

    bool header = true;
    for (const auto& arr : t) {
        if (header) {
            std::cout << "\033[1m";
            header = false;
        }

        for (int i = 0; i < arr.size(); i++) {
            std::cout << std::setw(maxLenghts[i] + 3) << std::left << arr[i];
        }
        std::cout << "\033[m" << std::endl;
    }
}