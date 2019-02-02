#ifndef ERROR_REPORTER_HPP
#define ERROR_REPORTER_HPP

#include <iostream>
#include <memory>

class ErrorStream
{
    class NewlinePrinter
    {
    public:
        ~NewlinePrinter();
    };

    std::shared_ptr< NewlinePrinter > _nlprntr;

public:
    ErrorStream() : _nlprntr(new NewlinePrinter()) {}
};

ErrorStream errors();

ErrorStream operator<<(ErrorStream(stream), const std::string &text);

#endif // ERROR_REPORTER_HPP
