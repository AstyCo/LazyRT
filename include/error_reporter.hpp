#include <iostream>

class ErrorStream
{
public:
    ~ErrorStream();
};

ErrorStream &errors();

ErrorStream &operator<<(ErrorStream &stream, const std::string &text);
