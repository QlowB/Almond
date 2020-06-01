#include "ExportRecipe.h"
#include "tinyxml2.h"

#include "XmlException.h"

using alm::ImageExportRecipe;
using alm::XmlException;


std::string ImageExportRecipe::toXml(void) const
{
}


ImageExportRecipe ImageExportRecipe::fromXml(const std::string& xml)
{
    tinyxml2::XMLDocument xmlDoc;
    xmlDoc.Parse(xml.c_str());
}
