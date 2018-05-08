#include "error_reporter.hpp"

ErrorStream::~ErrorStream()
{
    std::cerr << std::endl;
}

ErrorStream &errors()
{
    return ErrorStream();
}

ErrorStream &operator<<(ErrorStream &stream, const std::string &text)
{
    std::cerr << text << ' ';
}
