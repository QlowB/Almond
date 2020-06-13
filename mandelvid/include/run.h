#pragma once
#ifndef MANDELVID_RUN_H
#define MANDELVID_RUN_H

#include <string>
#include "Mandel.h"


void listGenerators(mnd::MandelContext& context);
void benchGenerators(mnd::MandelContext& context);
void renderImage(const std::string& xmlPath, const std::string& outPath);


#endif // MANDELVID_RUN_H
