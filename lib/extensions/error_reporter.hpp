#ifndef ERROR_REPORTER_HPP
#define ERROR_REPORTER_HPP

#include <iostream>
#include <memory>
#include <utility> // std::forward

class ErrorStream
{
    class NewlinePrinter
    {
    public:
        ~NewlinePrinter();
    };

    std::unique_ptr< NewlinePrinter > _nlprntr;

public:
    ErrorStream();
};

ErrorStream errors();

template < typename T >
ErrorStream operator<<(ErrorStream stream, T &&var)
{
    std::cerr << std::forward< T >(var) << ' ';
    return stream;
}

#endif // ERROR_REPORTER_HPP
