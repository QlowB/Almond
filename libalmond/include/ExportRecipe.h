#pragma once
#ifndef LIBALMOND_EXPORT_RECIPE_H
#define LIBALMOND_EXPORT_RECIPE_H

#include <MandelUtil.h>
#include "Gradient.h"

#include <string>

namespace alm
{
    struct ImageExportRecipe;
}


struct alm::ImageExportRecipe
{
    mnd::MandelInfo view;
    Gradient gradient;

    std::string toXml(void) const;
    static ImageExportRecipe fromXml(const std::string& xml);
};

#endif // LIBALMOND_EXPORT_RECIPE_H

