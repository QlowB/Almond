#include "run.h"

#include "Mandel.h"

#include "Serialize.h"
#include "ImageExport.h"

#include <fstream>
#include <iostream>

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