#include "extensions/help_functions.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h> // file_size
#include <fileapi.h> // file_size
#else // POSIX

#endif

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
        return std::pair<char *, long>(nullptr, 0);
    }
    long long fsize = file_size(fname);
    char *data = new char[fsize + 1];
    size_t read_count = fread(data, sizeof(char), fsize, file);
    fclose(file);

    return std::make_pair(data, read_count);
}

std::vector<char> strToVChar(const std::__cxx11::string &str)
{
    std::vector<char> result;
    std::copy(str.begin(), str.end(),
              std::back_inserter(result));
    return result;
}

Profiler::Profiler(bool verbal)
    : _verbal(verbal)
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
    if (_verbal)
        std::cout << eventName << " CPU time: " << newTime - _startTime << std::endl;

    _startTime = newTime;
}

void Profiler::finish(const std::string &eventName)
{
    MY_ASSERT(_started);
    step(eventName);
    _started = false;
}

long long file_size(const char *fname)
{
#ifdef WIN32
    HANDLE hFile = CreateFile(fname, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile==INVALID_HANDLE_VALUE)
        return -1; // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return -1; // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return size.QuadPart;
#else // POSIX

#endif
}

std::string makeIndents(int indent, int extra_spaces)
{
    std::string strIndents;
    for (int i = 0; i < indent; ++i)
        strIndents.push_back('\t');
    for (int i = 0; i < extra_spaces; ++i)
        strIndents.push_back('-');

    return strIndents;
}

std::vector<std::__cxx11::string> split(const std::__cxx11::string &str, const std::__cxx11::string &delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos)
            pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty())
            tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}
