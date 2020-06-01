#pragma once
#ifndef LIBALMOND_XMLEXCEPTION_H
#define LIBALMOND_XMLEXCEPTION_H

#include <stdexcept>
#include <string>

namespace alm
{
    struct XmlException;
}

struct alm::XmlException :
    std::runtime_error
{
    XmlException(const std::string& err);
};


#endif // LIBALMOND_XMLEXCEPTION_H
