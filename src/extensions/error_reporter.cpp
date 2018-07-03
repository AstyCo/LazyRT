#include "extensions/error_reporter.hpp"

ErrorStream::~ErrorStream()
{
    if (!_written)
        std::cerr << std::endl;
}

ErrorStream errors()
{
    return ErrorStream();
}

ErrorStream operator<<(ErrorStream stream, const std::string &text)
{
    std::cerr << text << ' ';
    stream._written = true;

    return stream;
}
