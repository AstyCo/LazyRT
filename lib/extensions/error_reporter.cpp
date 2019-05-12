#include "extensions/error_reporter.hpp"

ErrorStream errors() { return ErrorStream(); }

ErrorStream::NewlinePrinter::~NewlinePrinter() { std::cerr << std::endl; }

ErrorStream::ErrorStream() : _nlprntr(new NewlinePrinter()) {}
