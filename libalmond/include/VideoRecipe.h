#ifndef ALMOND_VIDEORECIPE_H
#define ALMOND_VIDEORECIPE_H

#include <optional>

#include "MandelUtil.h"
#include "Gradient.h"

namespace alm
{
    enum class Interpolation
    {
        NONE,
        LINEAR,
        SINE
    };

    template<typename T>
    struct Change
    {
        T newValue;
        Interpolation interpolation;
    };

    template<typename T>
    using MaybeChange = std::optional<Change<T>>;

    struct VideoPoint;
    struct VideoRecipe;
}


struct alm::VideoPoint
{
    MaybeChange<int> maxIterations;
    MaybeChange<mnd::MandelViewport> view;
    MaybeChange<Gradient> gradient;
};


class alm::VideoRecipe
{
    int width;
    int height;
};

#endif // ALMOND_VIDEORECIPE_H
