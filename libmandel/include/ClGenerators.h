#ifndef MANDEL_CLGENERATORS_H
#define MANDEL_CLGENERATORS_H

#ifdef WITH_OPENCL

#include "Generators.h"
#include "OpenClCode.h"

#define CL_TARGET_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl2.hpp>
#endif

namespace mnd
{
    class MandelDevice;

    class ClGenerator;
    class ClGeneratorFloat;
    class ClGeneratorDoubleFloat;
    class ClGeneratorDouble;
    class ClGeneratorDoubleDouble;
    class ClGeneratorQuadDouble;
    class ClGenerator128;
    class ClGenerator64;
}


class mnd::ClGenerator : public MandelGenerator
{
protected:
    MandelDevice& device;
    cl::Context& context;
    cl::Program program;
    cl::CommandQueue queue;
    cl::Kernel kernel;
public:
    ClGenerator(MandelDevice& device, const std::string& source, mnd::Precision type);
    virtual ~ClGenerator(void);

    virtual void generate(const MandelInfo& info, float* data) = 0;
    virtual mnd::MandelDevice* getDevice(void);
};


class mnd::ClGeneratorFloat : public ClGenerator
{
    bool useVec;
public:
    ClGeneratorFloat(MandelDevice& device, const std::string& code = getFloat_cl());
    virtual ~ClGeneratorFloat(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorDoubleFloat : public ClGenerator
{
public:
    ClGeneratorDoubleFloat(MandelDevice& device);
    virtual ~ClGeneratorDoubleFloat(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorDouble : public ClGenerator
{
public:
    ClGeneratorDouble(mnd::MandelDevice& device, const std::string& source = getDouble_cl());
    virtual ~ClGeneratorDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorDoubleDouble : public ClGenerator
{
    bool smooth;
public:
    ClGeneratorDoubleDouble(mnd::MandelDevice& device);
    virtual ~ClGeneratorDoubleDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGeneratorQuadDouble : public ClGenerator
{
    bool smooth;
public:
    ClGeneratorQuadDouble(mnd::MandelDevice& device);
    virtual ~ClGeneratorQuadDouble(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGenerator128 : public ClGenerator
{
public:
    ClGenerator128(mnd::MandelDevice& device);
    virtual ~ClGenerator128(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};


class mnd::ClGenerator64 : public ClGenerator
{
public:
    ClGenerator64(mnd::MandelDevice& device);
    virtual ~ClGenerator64(void) = default;

    virtual void generate(const MandelInfo& info, float* data) override;
protected:
    virtual std::string getKernelCode(bool smooth) const;
};

#endif // WITH_OPENCL

#endif // MANDEL_CLGENERATORS_H
