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
    void serialize(tinyxml2::XMLElement* elem, const T&);

    template<typename T>
    std::string toXml(const T&);

    template<typename T>
    T deserialize(tinyxml2::XMLElement* xml);

    template<typename T>
    T fromXml(const std::string& xml);


    // specializations
    template<>
    void serialize<Gradient>(tinyxml2::XMLElement* elem, const Gradient&);
    template<>
    Gradient deserialize<Gradient>(tinyxml2::XMLElement* xml);
    template<>
    std::string toXml<Gradient>(const Gradient&);
    template<>
    Gradient fromXml<Gradient>(const std::string& xml);


    template<>
    void serialize<mnd::MandelViewport>(tinyxml2::XMLElement* elem, const mnd::MandelViewport&);
    template<>
    mnd::MandelViewport deserialize<mnd::MandelViewport>(tinyxml2::XMLElement* xml);

    template<>
    void serialize<mnd::MandelInfo>(tinyxml2::XMLElement* elem, const mnd::MandelInfo&);
    template<>
    mnd::MandelInfo deserialize<mnd::MandelInfo>(tinyxml2::XMLElement* xml);

    template<>
    void serialize<ImageView>(tinyxml2::XMLElement* elem, const ImageView&);
    template<>
    ImageView deserialize<ImageView>(tinyxml2::XMLElement* xml);
    template<>
    std::string toXml<ImageView>(const ImageView&);
    template<>
    ImageView fromXml<ImageView>(const std::string& xml);
}


struct alm::ImageView
{
    mnd::MandelInfo view;
    Gradient gradient;
};

#endif // LIBALMOND_SERIALIZE_H
