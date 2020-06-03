#pragma once
#ifndef LIBALMOND_SERIALIZE_H
#define LIBALMOND_SERIALIZE_H

#include <MandelUtil.h>
#include "Gradient.h"

#include <string>

namespace tinyxml2
{
    struct XMLElement;
}

namespace alm
{
    struct ImageView;

    Gradient deserializeGradient(tinyxml2::XMLElement* xml);
    std::unique_ptr<tinyxml2::XMLElement> serializeGradient(const Gradient& g);

    Gradient loadGradient(const std::string& xml);
    std::string saveGradient(const Gradient& g);

    ImageView deserializeImageView(tinyxml2::XMLElement* xml);
    std::unique_ptr<tinyxml2::XMLElement> serializeImageView(const ImageView& iv);

    ImageView loadImageView(const std::string& xml);
    std::string saveImageView(const ImageView& iv);
}


struct alm::ImageView
{
    mnd::MandelInfo view;
    Gradient gradient;

    std::string toXml(void) const;
    static ImageView fromXml(const std::string& xml);
};

#endif // LIBALMOND_SERIALIZE_H
