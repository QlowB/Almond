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
using tinyxml2::XMLPrinter;
using tinyxml2::XML_SUCCESS;


template<>
alm::Gradient alm::deserialize<alm::Gradient>(XMLElement* elem)
{
    if (elem == nullptr)
        throw alm::XmlException{ "invalid root node" };

    bool repeat = false;
    bool hasMax = false;
    float maxVal = 0.0f;
    repeat = elem->BoolAttribute("repeat", true);
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


template<>
void alm::serialize<alm::Gradient>(tinyxml2::XMLElement* gradient, const alm::Gradient& grad)
{
    gradient->SetAttribute("repeat", grad.isRepeat());
    gradient->SetAttribute("version", "1.0.0");
    for (const auto& [col, point] : grad.getPoints()) {
        XMLElement* cElem = gradient->InsertNewChildElement("color");
        cElem->SetAttribute("r", int(col.r));
        cElem->SetAttribute("g", int(col.g));
        cElem->SetAttribute("b", int(col.b));
        cElem->SetAttribute("p", float(point));
    }
}


template<>
alm::Gradient alm::fromXml<alm::Gradient>(const std::string& xml)
{
    XMLDocument xmlDoc;
    XMLError err = xmlDoc.Parse(xml.c_str());
    if (err != XML_SUCCESS)
        throw alm::XmlException{ "error parsing gradient xml" };
    return deserialize<Gradient>(xmlDoc.RootElement());
}


template<>
std::string alm::toXml<alm::Gradient>(const alm::Gradient& g)
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


template<>
void alm::serialize<mnd::MandelViewport>(XMLElement* elem, const mnd::MandelViewport& v)
{
    XMLElement* x = elem->InsertNewChildElement("x");
    XMLElement* y = elem->InsertNewChildElement("y");
    XMLElement* w = elem->InsertNewChildElement("width");
    XMLElement* h = elem->InsertNewChildElement("height");
    x->SetText(mnd::toString(v.x).c_str());
    y->SetText(mnd::toString(v.y).c_str());
    w->SetText(mnd::toString(v.width).c_str());
    h->SetText(mnd::toString(v.height).c_str());
}


template<>
mnd::MandelViewport alm::deserialize<mnd::MandelViewport>(XMLElement* elem)
{
    if (elem == nullptr)
        throw alm::XmlException{ "invalid root node" };
    XMLElement* x = elem->FirstChildElement("x");
    XMLElement* y = elem->FirstChildElement("y");
    XMLElement* width = elem->FirstChildElement("width");
    XMLElement* height = elem->FirstChildElement("height");

    const char* xStr = x->GetText();
    const char* yStr = y->GetText();
    const char* wStr = width->GetText();
    const char* hStr = height->GetText();

    mnd::MandelViewport viewport;
    try {
        viewport.x = mnd::Real{ xStr };
        viewport.y = mnd::Real{ yStr };
        viewport.width = mnd::Real{ wStr };
        viewport.height = mnd::Real{ hStr };
    }
    catch(std::runtime_error& re) {
        throw XmlException{ "could not parse viewport" };
    }
    return viewport;
}


template<>
void alm::serialize<mnd::MandelInfo>(XMLElement* elem, const mnd::MandelInfo& v)
{
    XMLElement* viewport = elem->InsertNewChildElement("viewport");
    serialize(viewport, v.view);
    if (v.julia) {
        XMLElement* julia = elem->InsertNewChildElement("julia");
        XMLElement* juliaX = julia->InsertNewChildElement("x");
        XMLElement* juliaY = julia->InsertNewChildElement("y");
        juliaX->SetText(mnd::toString(v.juliaX).c_str());
        juliaY->SetText(mnd::toString(v.juliaY).c_str());
    }
    XMLElement* smooth = elem->InsertNewChildElement("smooth");
    XMLElement* maxIter = elem->InsertNewChildElement("maxIterations");
    XMLElement* width = elem->InsertNewChildElement("width");
    XMLElement* height = elem->InsertNewChildElement("height");
    smooth->SetText(v.smooth);
    maxIter->SetText(v.maxIter);
    width->SetText(v.bWidth);
    height->SetText(v.bHeight);
}


template<>
mnd::MandelInfo alm::deserialize<mnd::MandelInfo>(XMLElement* elem)
{
    if (elem == nullptr)
        throw alm::XmlException{ "invalid root node" };

    mnd::MandelInfo mi;
    XMLElement* viewport = elem->FirstChildElement("viewport");
    mi.view = deserialize<mnd::MandelViewport>(viewport);

    XMLElement* julia = elem->FirstChildElement("julia");
    if (julia != nullptr) {
        XMLElement* juliaX = julia->FirstChildElement("x");
        XMLElement* juliaY = julia->FirstChildElement("y");
        try {
            if (juliaX != nullptr) {
                mi.juliaX = mnd::Real{ juliaX->GetText() };
            }
            if (juliaY != nullptr) {
                mi.juliaY = mnd::Real{ juliaY->GetText() };
            }
        } catch (std::runtime_error& re) {
            throw XmlException{ "invalid julia parameters" };
        }
    }

    XMLElement* smooth = elem->FirstChildElement("smooth");
    if (smooth != nullptr) {
        mi.smooth = smooth->BoolText(true);
    }
    else {
        mi.smooth = false;
    }

    XMLElement* bWidth = elem->FirstChildElement("width");
    XMLElement* bHeight = elem->FirstChildElement("height");
    if (bWidth != nullptr && bHeight != nullptr) {
        mi.bWidth = long(bWidth->Int64Text(1920));
        mi.bHeight = long(bHeight->Int64Text(1080));
    }
    else {
        mi.bWidth = 1920;
        mi.bHeight = 1080;
    }

    XMLElement* maxIter = elem->FirstChildElement("maxIterations");
    if (maxIter != nullptr) {
        mi.maxIter = maxIter->Int64Text(500);
    }
    else {
        mi.maxIter = 500;
    }
    return mi;
}


template<>
void alm::serialize<alm::ImageView>(XMLElement* elem, const alm::ImageView& v)
{
    XMLElement* view = elem->InsertNewChildElement("view");
    XMLElement* gradient = elem->InsertNewChildElement("gradient");
    serialize(view, v.view);
    serialize(gradient, v.gradient);
}


template<>
alm::ImageView alm::deserialize<alm::ImageView>(XMLElement* elem)
{
    if (elem == nullptr)
        throw alm::XmlException{ "invalid root node" };
    XMLElement* view = elem->FirstChildElement("view");
    XMLElement* gradient = elem->FirstChildElement("gradient");

    ImageView iv;
    iv.view = deserialize<mnd::MandelInfo>(view);

    if (gradient != nullptr) {
        iv.gradient = deserialize<alm::Gradient>(gradient);
    }
    else {
        iv.gradient = alm::Gradient::defaultGradient();
    }
    return iv;
}


template<>
std::string alm::toXml<alm::ImageView>(const ImageView& iv)
{
    XMLDocument doc;
    XMLElement* imageView = doc.NewElement("imageView");
    serialize(imageView, iv);
    doc.InsertFirstChild(imageView);
    XMLPrinter printer;
    doc.Accept(&printer);
    return printer.CStr();
}


template<>
alm::ImageView alm::fromXml<ImageView>(const std::string& xml)
{
    XMLDocument xmlDoc;
    XMLError err = xmlDoc.Parse(xml.c_str());
    if (err != XML_SUCCESS)
        throw alm::XmlException{ "error parsing gradient xml" };
    return deserialize<ImageView>(xmlDoc.RootElement());
}




