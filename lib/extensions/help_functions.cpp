#include "extensions/help_functions.hpp"

#include "command_line_args.hpp"

#include <iostream>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>     // file_size
#include <fileapi.h>     // file_size
#include <io.h>          // access
#define access _access_s // access
#else                    // POSIX
#include <sys/stat.h>    // file_size
#include <unistd.h>      // access
#endif

static bool is_file_exists(const char *fname) { return access(fname, 0) == 0; }

std::pair< char *, long > readBinaryFile(const char *fname)
{
    return readFile(fname, "rb");
}

std::pair< char *, long > readFile(const char *fname, const char *mode)
{
    if (!is_file_exists(fname)) {
        if (clargs.verbal())
            errors() << "file" << std::string(fname) << "doesn't exist";
        return std::pair< char *, long >(nullptr, 0);
    }
    FILE *file = fopen(fname, mode);
    if (!file) {
        if (clargs.verbal())
            errors() << "file" << std::string(fname) << "can not be opened";
        return std::pair< char *, long >(nullptr, 0);
    }
    long long fsize = file_size(fname);
    char *data = new char[fsize + 1];
    size_t read_count = fread(data, sizeof(char), fsize, file);
    fclose(file);

    return std::make_pair(data, read_count);
}

std::vector< char > strToVChar(const std::string &str)
{
    std::vector< char > result;
    std::copy(str.begin(), str.end(), std::back_inserter(result));
    return result;
}

Profiler::Profiler(bool verbal) : _verbal(verbal) { start(); }

void Profiler::start()
{
    _started = true;
    _startTime = getCpuTime();
}

void Profiler::step(const std::string &eventName)
{
    if (!_started)
        return;
    double newTime = getCpuTime();
    if (_verbal && !eventName.empty())
        std::cout << eventName << " CPU time: " << newTime - _startTime
                  << std::endl;

    _startTime = newTime;
}

void Profiler::finish(const std::string &eventName)
{
    assert(_started);
    step(eventName);
    _started = false;
}

long long file_size(const char *fname)
{
#ifdef WIN32
    HANDLE hFile =
        CreateFile(fname, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1; // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1; // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return size.QuadPart;
#else // POSIX
    struct stat statbuf;

    if (stat(fname, &statbuf) == -1) {
        /* check the value of errno */
        assert(false);
        return 0;
    }
    return statbuf.st_size;
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

std::vector< std::string > split(const std::string &str,
                                 const std::string &delim)
{
    std::vector< std::string > tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos)
            pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty())
            tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

char osSeparator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

void writeBinaryFile(const char *fname, const void *data, size_t type_size,
                     size_t length)
{
    FILE *pFile = fopen(fname, "wb");
    if (!pFile) {
        errors() << "ERROR: Write:: Failed to open file" << std::string(fname);
        return;
    }

    fwrite(data, type_size, length, pFile);
    fclose(pFile);
}
