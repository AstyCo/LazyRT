#include "extensions/help_functions.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>

void Asserter(const char *file, int line)
{
    std::cerr << "ASSERT at FILE:" << file << " LINE:"<< line << std::endl;
    exit(1);
}

std::pair<char *, long> readBinaryFile(const char *fname)
{
    return readFile(fname, "rb");
}

std::pair<char *, long> readFile(const char *fname, const char *mode)
{
    FILE* file = fopen(fname, mode);
    if (!file) {
        errors() << "file" << std::string(fname) << "can not be opened";
        return std::pair<char *, long>(NULL, 0);
    }
    fseek(file, 0L, SEEK_END);
    int length = ftell(file);
    fseek(file, 0L, SEEK_SET);
    char *data = new char[length + 1];
    fread(data, sizeof(char), length, file);
    fclose(file);
    data[length] = 0;

    return std::make_pair(data, length);
}

std::vector<char> strToVChar(const std::__cxx11::string &str)
{
    std::vector<char> result;
    std::copy(str.begin(), str.end(),
              std::back_inserter(result));
    return result;
}

Profiler::Profiler()
{
    start();
}

void Profiler::start()
{
    _started = true;
    _startTime = getCpuTime();
}

void Profiler::step(const std::string &eventName)
{
    if (!_started || eventName.empty())
        return;
    double newTime = getCpuTime();

    std::cout << eventName << " CPU time: " << newTime - _startTime << std::endl;

    _startTime = newTime;
}

void Profiler::finish(const std::string &eventName)
{
    MY_ASSERT(_started);
    step(eventName);
    _started = false;
}
