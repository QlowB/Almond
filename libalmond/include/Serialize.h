#pragma once
#ifndef LIBALMOND_SERIALIZE_H
#define LIBALMOND_SERIALIZE_H

#include <MandelUtil.h>
#include "Gradient.h"

#include <string>

namespace tinyxml2
{
    class XMLElement;
    class XMLDocument;
}


namespace alm
{
    struct ImageView;


    template<typename T>
    tinyxml2::XMLElement* serialize(tinyxml2::XMLDocument& doc, const T&);

    template<typename T>
    std::string toXml(const T&);

    template<typename T>
    T deserialize(tinyxml2::XMLElement* xml);

    template<typename T>
    T fromXml(const std::string& xml);


    // specializations
    template<>
    tinyxml2::XMLElement* serialize<Gradient>(tinyxml2::XMLDocument& doc, const Gradient&);
    template<>
    std::string toXml<Gradient>(const Gradient&);
    template<>
    Gradient deserialize<Gradient>(tinyxml2::XMLElement* xml);
    template<>
    Gradient fromXml<Gradient>(const std::string& xml);
}


struct alm::ImageView
{
    mnd::MandelInfo view;
    Gradient gradient;

    std::string toXml(void) const;
    static ImageView fromXml(const std::string& xml);
};

#endif // LIBALMOND_SERIALIZE_H
