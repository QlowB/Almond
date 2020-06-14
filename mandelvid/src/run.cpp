#include "run.h"

#include "Util.h"

#include "Serialize.h"
#include "ImageExport.h"

#include "Benchmark.h"

#include <fstream>
#include <iostream>
#include <map>



void listGenerators(mnd::MandelContext& context)
{
    Table cputable;
    cputable.push_back({ "Data Type", "Extension", "Precision" });
    for (const auto& [type, extension] : context.getSupportedTypes()) {
        cputable.push_back({ mnd::toString(type), mnd::toString(extension), mnd::toLegibleString(mnd::getPrecision(type)) });
    }

    std::cout << "CPU: " << context.getCpuInfo().getBrand() << std::endl;
    printTable(cputable);

    for (auto& device : context.getDevices()) {
        Table devtable;
        devtable.push_back({ "Data Type", "Extension", "Precision" });
        for (const auto& type : device->getSupportedTypes()) {
            devtable.push_back({ mnd::toString(type), "", mnd::toLegibleString(mnd::getPrecision(type)) });
        }
        std::cout << std::endl;
        std::cout << "Device: [" << device->getVendor() << "] " << device->getName() << std::endl;
        printTable(devtable);
    }

}


void benchGenerators(mnd::MandelContext& context)
{
    Table table;
    table.push_back({ "Device", "Data Type", "Extension", "Precision", "Performance [MI/s]" });
    std::map<size_t, mnd::MandelGenerator*> generators;
    for (const auto& [type, extension] : context.getSupportedTypes()) {
        generators[table.size()] = context.getCpuGenerator(type, extension);
        table.push_back({ context.getCpuInfo().getBrand(), mnd::toString(type), mnd::toString(extension), mnd::toLegibleString(mnd::getPrecision(type)) });
    }

    for (auto& device : context.getDevices()) {
        for (const auto& type : device->getSupportedTypes()) {
            generators[table.size()] = device->getGenerator(type);
            table.push_back({ device->getName(), mnd::toString(type), "", mnd::toLegibleString(mnd::getPrecision(type)) });
        }
    }

    std::map<size_t, size_t> maxLenghts;

    for (const auto& arr : table) {
        for (int i = 0; i < arr.size(); i++) {
            if (arr[i].size() > maxLenghts[i])
                maxLenghts[i] = arr[i].size();
        }
    }
    std::cout << "\033[1m";
    for (int i = 0; i < table.size(); i++) {
        const auto& arr = table[i];
        for (int i = 0; i < arr.size(); i++) {
            std::cout << std::setw(maxLenghts[i] + 3) << std::left << arr[i];
        }
        if (i > 0) {
            double iterPerNanos = mnd::benchmark(*generators[i]);
            std::cout << std::fixed << std::setprecision(2) << (iterPerNanos * 1000);
        }
        std::cout << "\033[m" << std::endl;
    }
}


std::string readFile(const std::string& path)
{
    std::ifstream inFile(path.c_str());
    return std::string{ std::istreambuf_iterator<char>(inFile),
        std::istreambuf_iterator<char>() };
}


void renderImage(const std::string& xmlPath, const std::string& outPath)
{
    mnd::MandelContext mndCtxt = mnd::initializeContext();
    std::string xml = readFile(xmlPath);
    alm::ImageView iv = alm::fromXml<alm::ImageView>(xml);

    alm::ImageExportInfo iei;
    iei.drawInfo = iv.view;
    iei.generator = &mndCtxt.getDefaultGenerator();
    iei.gradient = iv.gradient;
    iei.path = outPath;

    exportImage(iei, [] (float progress) {
        std::cout << progress << "% completed" << std::endl;
    });
}
