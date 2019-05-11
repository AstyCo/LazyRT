#include "help_functions.hpp"
#include <command_line_args.hpp>
#include <types/splitted_string.hpp>

#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm> // any_of

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h> // _mkdir (win32), mkdir

#ifdef WIN32
#include <windows.h>     // file_size
#include <fileapi.h>     // file_size
#include <io.h>          // access
#define access _access_s // access
#else                    // POSIX
#include <sys/stat.h>    // file_size
#include <unistd.h>      // access
#endif

FileData readBinaryFile(const char *fname) { return readFile(fname, "rb"); }

FileData readFile(const char *fname, const char *mode)
{
    if (!exists(fname)) {
        if (clargs.verbal())
            errors() << "file" << std::string(fname) << "doesn't exist";
        return FileData();
    }
    FILE *file = fopen(fname, mode);
    if (!file) {
        if (clargs.verbal())
            errors() << "file" << std::string(fname) << "can not be opened";
        return FileData();
    }
    long long fsize = file_size(fname);
    std::shared_ptr< char > data(new char[fsize + 1]);
    size_t read_count = fread(data.get(), sizeof(char), fsize, file);
    fclose(file);

    return FileData(data, read_count);
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

bool checkPatterns(const std::string &str,
                   const std::vector< std::string > &patterns)
{
    return std::any_of(patterns.begin(), patterns.end(),
                       [str](const std::string &pattern) {
                           return str_contains(str, pattern);
                       });
}

bool is_directory(const char *path)
{
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

bool is_directory(const SplittedPath &sp)
{
    return is_directory(sp.jointOs().c_str());
}

bool is_file(const char *path)
{
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool is_file(const SplittedPath &sp) { return is_file(sp.jointOs().c_str()); }

bool exists(const char *path) { return is_file(path) || is_directory(path); }

bool exists(const SplittedPath &sp) { return exists(sp.jointOs().c_str()); }

std::string extension(const std::string &filename)
{
    auto pos = filename.rfind('.');
    if (pos == std::string::npos)
        return std::string();
    // check for filenames starting with dot
    if (pos == 0 || is_separator(filename[pos - 1]))
        return std::string();
    std::string ext = filename.substr(pos);
    // check for path/to/inner.dot/file (".dot/file")
    for (auto it = ext.cbegin(); it != ext.cend(); ++it) {
        const auto &ch = *it;
        if (is_separator(ch))
            return std::string();
    }
    return ext;
}

std::string extension(const SplittedPath &sp)
{
    return extension(sp.jointOs());
}

void create_directories(const std::string &path)
{
    create_directories(SplittedPath(path, SplittedPath::osSep()));
}

void create_directories(const SplittedPath &sp)
{
    SplittedPath currentPath;
    currentPath.setOsSeparator();
    for (const auto &filename : sp.splitted()) {
        currentPath.append(filename);
        create_directory(currentPath);
    }
}

void create_directory(const std::string &path)
{
    int nError = 0;
#if defined(_WIN32)
    nError = _mkdir(path.c_str()); // can be used on Windows
#else
    mode_t nMode = 0733;                 // UNIX style permissions
    nError = mkdir(path.c_str(), nMode); // can be used on non-Windows
#endif
}

void create_directory(const SplittedPath &sp)
{
    create_directory(sp.jointOs());
}
