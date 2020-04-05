#include "Generators.h"

#include <cstdio>

using mnd::MandelGenerator;
using mnd::AdaptiveGenerator;


MandelGenerator::~MandelGenerator(void)
{
}



mnd::MandelDevice* MandelGenerator::getDevice(void)
{
    return nullptr;
}


mnd::Real MandelGenerator::getPrecision(void) const
{
    return precision;
}


mnd::Precision MandelGenerator::getType(void) const
{
    return type;
}


mnd::CpuExtension MandelGenerator::getExtension(void) const
{
    return extension;
}


AdaptiveGenerator::AdaptiveGenerator(void) :
    MandelGenerator{ mnd::Precision::INF_PREC },
    generators{}
{
}


AdaptiveGenerator::AdaptiveGenerator(MandelGenerator* floatGen, MandelGenerator* doubleGen) :
    AdaptiveGenerator{}
{
    generators.insert({ 0.0000001, floatGen });
    generators.insert({ 0.0, doubleGen });
}

/*
AdaptiveGenerator::AdaptiveGenerator(Generator* floatGen, Generator* doubleGen, Generator* quadGen)
{
    generators.insert({ 0.0000001, floatGen });
    generators.insert({ 5.0e-16, doubleGen });
    generators.insert({ Real("1.0e-28"), quadGen });
}*/


void AdaptiveGenerator::addGenerator(const mnd::Real& precision, mnd::MandelGenerator& generator)
{
    generators.insert({ precision, &generator });
}


void AdaptiveGenerator::addGenerator(mnd::Precision p, MandelGenerator& generator)
{
    generators.insert({ mnd::getPrecision(p), &generator });
}


void AdaptiveGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    Real pixelW = info.view.width / info.bWidth;
    Real pixelH = info.view.height / info.bHeight;
    Real neededPrecision = pixelW < pixelH ? pixelW : pixelH;

    //Generator* toUse = nullptr;
    auto firstSmaller = generators.lower_bound(neededPrecision);
    if (firstSmaller != generators.end()) {
        //printf("use generator with precision: %s\n", mnd::toString(firstSmaller->first).c_str());
        //printf("gen: %p\n", firstSmaller->second);fflush(stdout);
        firstSmaller->second->generate(info, data);
    }
    else {
        for (long s = 0; s < info.bWidth * info.bHeight; s++) {
            data[s] = 0.0;
        }
    }
    return;

/*
    int i = 0;

    for (auto [thresh, gen] : generators) {
        ++i;
        if (neededPrecision > thresh) {
            toUse = gen;
        }
    }
    if (toUse != nullptr) {
        toUse->generate(info, data);
    }
    else {
        for (long s = 0; s < info.bWidth * info.bHeight; s++) {
            data[s] = 0.0;
        }
    }*/
}


namespace mnd
{
    std::string toString(Precision p)
    {
        switch (p) {
        case Precision::FLOAT:
            return "float";
        case Precision::DOUBLE_FLOAT:
            return "double-float";
        case Precision::DOUBLE:
            return "double";
        case Precision::DOUBLE_DOUBLE:
            return "double-double";
        case Precision::FLOAT128:
            return "float128";
        case Precision::FLOAT256:
            return "float256";
        case Precision::FLOAT512:
            return "float512";
        case Precision::FIXED64:
            return "fixed64";
        case Precision::FIXED128:
            return "fixed128";
        case Precision::FIXED512:
            return "fixed512";
        case Precision::QUAD_DOUBLE:
            return "quad-double";
        case Precision::INF_PREC:
            return "real";
        }
        return "";
    }


    std::string toString(CpuExtension ce)
    {
        switch (ce) {
        case CpuExtension::NONE:
            return "";
        case CpuExtension::X86_SSE2:
            return "SSE2";
        case CpuExtension::X86_AVX:
            return "AVX";
        case CpuExtension::X86_AVX_FMA:
            return "AVX2+FMA";
        case CpuExtension::X86_AVX_512:
            return "AVX512";
        case CpuExtension::ARM_NEON:
            return "NEON";
        }
        return "";
    }


    Real getPrecision(Precision p)
    {
        static const std::map<Precision, Real> precs {
            { Precision::FLOAT, getPrecision<float>() },
            { Precision::DOUBLE_FLOAT, Real("4.0e-15") },
            { Precision::DOUBLE, getPrecision<double>() },
            { Precision::DOUBLE_DOUBLE, Real("1.0e-29") },
            { Precision::QUAD_DOUBLE, Real("1.0e-56") },
            { Precision::FIXED64, Real("3.5e-15") },
            { Precision::FIXED128, Real("1.317e-29") },
            { Precision::FIXED512, Real("1.5e-130") },
            { Precision::FLOAT128, Real("1.5e-29") },
            { Precision::FLOAT256, Real("1.0e-58") },
            { Precision::FLOAT512, Real("1.0e-145") },
            { Precision::INF_PREC, Real(0.0) },
        };

        return precs.at(p);
    }

    Real getPrecision(GeneratorType t)
    {
        switch(t) {
        case GeneratorType::FLOAT:
        case GeneratorType::FLOAT_SSE2:
        case GeneratorType::FLOAT_AVX:
        case GeneratorType::FLOAT_AVX_FMA:
        case GeneratorType::FLOAT_AVX512:
        case GeneratorType::FLOAT_NEON:
            return getPrecision<float>();
        case GeneratorType::DOUBLE_FLOAT:
            return getPrecision(Precision::DOUBLE_FLOAT);
        case GeneratorType::DOUBLE:
        case GeneratorType::DOUBLE_SSE2:
        case GeneratorType::DOUBLE_AVX:
        case GeneratorType::DOUBLE_AVX_FMA:
        case GeneratorType::DOUBLE_AVX512:
        case GeneratorType::DOUBLE_NEON:
            return getPrecision<double>();
        case GeneratorType::DOUBLE_DOUBLE:
        case GeneratorType::DOUBLE_DOUBLE_AVX:
        case GeneratorType::DOUBLE_DOUBLE_AVX_FMA:
            return getPrecision<DoubleDouble>();
        case GeneratorType::QUAD_DOUBLE:
            return getPrecision<QuadDouble>();
        case GeneratorType::FLOAT128:
            return getPrecision<Float128>();
        case GeneratorType::FLOAT256:
            return getPrecision<Float256>();
        case GeneratorType::FIXED64:
            return getPrecision<Fixed64>();
        case GeneratorType::FIXED128:
            return getPrecision<Fixed128>();
        case GeneratorType::FIXED512:
            return getPrecision<Fixed512>();

        case GeneratorType::UNSPECIFIED:
        default:
            return Real(0);
        }
    }


    template<>
    Real getPrecision<float>() {
        return Real("1.0e-7");
    }
    template<>
    Real getPrecision<double>() {
        return Real("1.0e-15");
    }
    template<>
    Real getPrecision<DoubleDouble>() {
        return Real("1.0e-29");
    }
    template<>
    Real getPrecision<QuadDouble>() {
        return Real("3.0e-64");
    }
    template<>
    Real getPrecision<Fixed64>() {
        return Real("3.5e-15");
    }
    template<>
    Real getPrecision<Fixed128>() {
        return Real("1.5e-29");
    }
    template<>
    Real getPrecision<Fixed512>() {
        return Real("1.5e-130");
    }
    template<>
    Real getPrecision<Float128>() {
        return Real("1.5e-29");
    }
    template<>
    Real getPrecision<Float256>() {
        return Real("1.4e-72");
    }
    template<>
    Real getPrecision<Float512>() {
        return Real("1.0e-145");
    }

}




