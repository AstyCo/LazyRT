#include "extensions/error_reporter.hpp"

ErrorStream errors()
{
    return ErrorStream();
}

ErrorStream operator<<(ErrorStream stream, const std::string &text)
{
    std::cerr << text << ' ';
    return stream;
}

ErrorStream::NewlinePrinter::~NewlinePrinter()
{
    std::cerr << std::endl;
}
