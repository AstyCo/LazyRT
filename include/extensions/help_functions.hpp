#ifndef LAZYRT_HELP_FUNCTIONS_HPP
#define LAZYRT_HELP_FUNCTIONS_HPP

#include "extensions/error_reporter.hpp"

#include <string>
#include <sstream>
#include <vector>

void Asserter(const char *file, int line);
#define MY_ASSERT(x) if (!(x)) Asserter(__FILE__, __LINE__);


template <typename T>
std::string numberToString ( T Number )
{
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

typedef unsigned int MurmurHashType;
MurmurHashType MurmurHash2 (const void *key, int len, unsigned int seed = 0);

double getWallTime();
double getCpuTime();

std::pair<char *, long> readBinaryFile(const char *fname);
std::pair<char *, long> readFile(const char *fname, const char *mode);

template <typename T>
void writeBinaryFile(const char *fname, T *data, size_t length)
{
    FILE * pFile = fopen (fname, "wb");
    if (!pFile) {
        errors() << "ERROR: Write:: Failed to open file" << std::string(fname);
        return;
    }

    fwrite(data , sizeof(T), length, pFile);
    fclose(pFile);
}

std::vector<char> strToVChar(const std::string &str);

inline char osSeparator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}


#endif // LAZYRT_HELP_FUNCTIONS_HPP