#ifndef MANDEL_CLGENERATORS_H
#define MANDEL_CLGENERATORS_H

#ifdef WITH_OPENCL

#include "Generators.h"

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

namespace mnd
{
    class ClGenerator;
    class ClGeneratorFloat;
    class ClGeneratorDouble;
    class ClGeneratorDoubleDouble;
    class ClGeneratorQuadDouble;
    class ClGenerator128;
}


class mnd::ClGenerator : public Generator
{
protected:
    cl::Device device;
    cl::Context context;
    cl::Program program;
    cl::CommandQueue queue;
public:
    ClGenerator(cl::Device device);
    virtual ~ClGenerator(void);

    virtual void generate(const MandelInfo& info, float* data);
protected:
    virtual std::string getKernelCode(bool smooth) const = 0;
};


class mnd::ClGeneratorFloat : public ClGenerator
{
public:
    ClGeneratorFloat(cl::Device device);
    virtual ~ClGeneratorFloat(void) = default;

protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorDouble : public ClGenerator
{
public:
    ClGeneratorDouble(cl::Device device);
    virtual ~ClGeneratorDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data);
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorDoubleDouble : public ClGenerator
{
    bool smooth;
public:
    ClGeneratorDoubleDouble(cl::Device device);
    virtual ~ClGeneratorDoubleDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data);
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorQuadDouble : public ClGenerator
{
    bool smooth;
public:
    ClGeneratorQuadDouble(cl::Device device);
    virtual ~ClGeneratorQuadDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data);
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGenerator128 : public ClGenerator
{
public:
    ClGenerator128(cl::Device device);
    virtual ~ClGenerator128(void) = default;

    virtual void generate(const MandelInfo& info, float* data);
protected:
    virtual std::string getKernelCode(bool smooth) const;
};

#endif // WITH_OPENCL

#endif // MANDEL_CLGENERATORS_H
