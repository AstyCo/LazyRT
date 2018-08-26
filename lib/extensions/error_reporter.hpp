#ifndef ERROR_REPORTER_HPP
#define ERROR_REPORTER_HPP

#include <iostream>

class ErrorStream
{
public:
    bool _written;
public:
    ErrorStream() : _written(false) {}
    ~ErrorStream();
};

ErrorStream errors();

ErrorStream operator<<(ErrorStream stream, const std::string &text);

#endif // ERROR_REPORTER_HPP
