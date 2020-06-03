#include "Serialize.h"
#include "tinyxml2.h"

#include "XmlException.h"

#include <sstream>

using alm::ImageView;
using alm::XmlException;

using tinyxml2::XMLElement;
using tinyxml2::XMLDocument;
using tinyxml2::XMLError;
using tinyxml2::XMLNode;
using tinyxml2::XML_SUCCESS;


alm::Gradient alm::deserializeGradient(XMLElement* elem)
{
    if (elem == nullptr)
        throw alm::XmlException{ "invalid root node" };

    bool repeat = false;
    bool hasMax = false;
    float maxVal = 0.0f;
    repeat = elem->BoolAttribute("repeat", false);
    XMLError e = elem->QueryFloatAttribute("max", &maxVal);
    if (e == XML_SUCCESS)
        hasMax = true;

    std::vector<std::pair<RGBColor, float>> points;

    XMLElement* colorNode = elem->FirstChildElement("color");
    while(colorNode != nullptr) {
        int r = colorNode->IntAttribute("r");
        int g = colorNode->IntAttribute("g");
        int b = colorNode->IntAttribute("b");
        float p = colorNode->FloatAttribute("p");
        
        uint8_t cr = uint8_t(std::clamp(r, 0, 255));
        uint8_t cg = uint8_t(std::clamp(g, 0, 255));
        uint8_t cb = uint8_t(std::clamp(b, 0, 255));
        points.push_back(std::make_pair(RGBColor{ cr, cg, cb }, p));

        colorNode = colorNode->NextSiblingElement("color");
    }
    if (hasMax)
        return Gradient{ std::move(points), maxVal, repeat };
    else
        return Gradient{ std::move(points), repeat };
}


alm::Gradient alm::loadGradient(const std::string& xml)
{
    XMLDocument xmlDoc;
    XMLError err = xmlDoc.Parse(xml.c_str());
    if (err != XML_SUCCESS)
        throw alm::XmlException{ "error parsing gradient xml" };
    return deserializeGradient(xmlDoc.RootElement());
}


std::string alm::saveGradient(const Gradient& g)
{
    std::stringstream buf;

    std::string version = "1.0.0";
    buf << "<gradient max=\"" << g.getMax() << "\" repeat=\"" << (g.isRepeat() ? "true" : "false") 
        << "\" version=\"" << version << "\" >" << std::endl;
    for (const auto&[color, val] : g.getPoints()) {
        buf << "    <color " <<
               "r=\"" << int(color.r) <<
               "\" g=\"" << int(color.g) <<
               "\" b=\"" << int(color.b) <<
               "\" p=\"" << val << "\" />" << std::endl;
    }
    buf << "</gradient>" << std::endl;
    return buf.str();
}


ImageView deserializeImageView(tinyxml2::XMLElement* xml);
std::unique_ptr<tinyxml2::XMLElement> serializeImageView(const ImageView& iv);
