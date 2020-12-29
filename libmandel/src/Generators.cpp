#include "Generators.h"

#include "Types.h"

#include <cstdio>

using mnd::MandelGenerator;
using mnd::AdaptiveGenerator;


namespace mnd
{
    template<> Real getPrecision<float>();
    template<> Real getPrecision<double>();
    template<> Real getPrecision<DoubleDouble>();
    template<> Real getPrecision<TripleDouble>();
    template<> Real getPrecision<QuadDouble>();
    template<> Real getPrecision<HexDouble>();
    template<> Real getPrecision<OctaDouble>();
    template<> Real getPrecision<Fixed64>();
    template<> Real getPrecision<Fixed128>();
    template<> Real getPrecision<Fixed512>();
    template<> Real getPrecision<Float128>();
    template<> Real getPrecision<Float256>();
    template<> Real getPrecision<Float512>();

    template<typename T>
    Precision getType(void);
    template<> Precision getType<float>() { return Precision::FLOAT; }
    template<> Precision getType<double>() { return Precision::DOUBLE; }
    template<> Precision getType<DoubleDouble>() { return Precision::DOUBLE_DOUBLE; }
    template<> Precision getType<TripleDouble>() { return Precision::TRIPLE_DOUBLE; }
    template<> Precision getType<QuadDouble>() { return Precision::QUAD_DOUBLE; }
    template<> Precision getType<HexDouble>() { return Precision::HEX_DOUBLE; }
    template<> Precision getType<OctaDouble>() { return Precision::OCTA_DOUBLE; }
    template<> Precision getType<Fixed64>() { return Precision::FIXED64; }
    template<> Precision getType<Fixed128>() { return Precision::FIXED128; }
    template<> Precision getType<Fixed512>() { return Precision::FIXED512; }
    template<> Precision getType<Float128>() { return Precision::FLOAT128; }
    template<> Precision getType<Float256>() { return Precision::FLOAT256; }
    template<> Precision getType<Float512>() { return Precision::FLOAT512; }
}



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


mnd::HardwareFeature MandelGenerator::getExtension(void) const
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


void AdaptiveGenerator::addGenerator(MandelGenerator& generator)
{
    generators.insert({ generator.getPrecision(), &generator });
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
        case Precision::TRIPLE_FLOAT:
            return "triple-float";
        case Precision::DOUBLE:
            return "double";
        case Precision::DOUBLE_DOUBLE:
            return "double-double";
        case Precision::TRIPLE_DOUBLE:
            return "triple-double";
        case Precision::QUAD_DOUBLE:
            return "quad-double";
        case Precision::HEX_DOUBLE:
            return "hex-double";
        case Precision::OCTA_DOUBLE:
            return "octa-double";
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
        case Precision::INF_PREC:
            return "real";
        }
        return "";
    }


    std::string toString(HardwareFeature ce)
    {
        switch (ce) {
        case HardwareFeature::NONE:
            return "";
        case HardwareFeature::X86_SSE2:
            return "SSE2";
        case HardwareFeature::X86_AVX:
            return "AVX";
        case HardwareFeature::X86_AVX_FMA:
            return "AVX2+FMA";
        case HardwareFeature::X86_AVX_512:
            return "AVX512";
        case HardwareFeature::ARM_NEON:
            return "NEON";
        }
        return "";
    }


    Real getPrecision(Precision p)
    {
        static const std::map<Precision, Real> precs {
            { Precision::FLOAT, getPrecision<float>() },
            { Precision::DOUBLE_FLOAT, Real("4.0e-15") },
            { Precision::TRIPLE_FLOAT, Real("1.0e-21") },
            { Precision::DOUBLE, getPrecision<double>() },
            { Precision::DOUBLE_DOUBLE, Real("1.0e-29") },
            { Precision::TRIPLE_DOUBLE, Real("1.0e-47") },
            { Precision::QUAD_DOUBLE, Real("1.0e-56") },
            { Precision::HEX_DOUBLE, Real("1.0e-94") },
            { Precision::OCTA_DOUBLE, Real("1.0e-126") },
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
    Real getPrecision<TripleDouble>() {
        return Real("1.0e-47");
    }
    template<>
    Real getPrecision<QuadDouble>() {
        return Real("3.0e-64");
    }
    template<>
    Real getPrecision<HexDouble>() {
        return Real("3.0e-94");
    }
    template<>
    Real getPrecision<OctaDouble>() {
        return Real("1.0e-126");
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




