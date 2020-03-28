#ifndef MANDEL_OPENCLINTERNAL_H
#define MANDEL_OPENCLINTERNAL_H

#ifdef WITH_OPENCL
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif
namespace mnd
{
    struct ClDeviceWrapper
    {
#ifdef WITH_OPENCL
        cl::Device device;
        cl::Context context;
#endif
    };
}


#endif // MANDEL_OPENCLINTERNAL_H


