#ifndef MANDEL_FLOATLOG_H
#define MANDEL_FLOATLOG_H

///
/// \file FloatLog.h
///
/// This file exists as a workaround to the problem that when accessing functions
/// in headers from a compilation unit which is compiled for a different
/// instruction set (e.g. avx), the linker may take the avx-version also for
/// calls from files compiled without avx.
///

///
/// \brief same as \code float logf(float) \endcode
///
float floatLog(float x);

///
/// \brief same as \code float log2f(float) \endcode
///
float floatLog2(float x);

#endif // MANDEL_FLOATLOG_H
