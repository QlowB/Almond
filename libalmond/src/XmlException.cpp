#include "XmlException.h"

using alm::XmlException;

XmlException::XmlException(const std::string& error) :
    std::runtime_error{ error }
{
}
