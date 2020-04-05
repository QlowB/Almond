#ifndef MANDEL_OPENCLINTERNAL_H
#define MANDEL_OPENCLINTERNAL_H

#define CL_TARGET_OPENCL_VERSION 220
#define CL_HPP_TARGET_OPENCL_VERSION 220
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

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


