#ifndef LAZYRT_HELP_FUNCTIONS_HPP
#define LAZYRT_HELP_FUNCTIONS_HPP

#include "lazyut_global.hpp"
#include "extensions/error_reporter.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <cassert>

#define MY_PRINTEXT(x)                                                         \
    std::cout << #x << " EXT_FILE : " << _currentFile->name()                  \
              << " LINE: " << _line << std::endl
#define ASSERT_F(x)                                                            \
    if (!(x)) {                                                                \
        std::cerr << "EXT_FILE : " << _currentFile->name()                     \
                  << " LINE: " << _line << std::endl;                          \
        assert(x);                                                             \
    }

#define VERBAL_0(x)
#define VERBAL_1(x)

#define COUNT_LINES(x)

template < typename T >
std::string numberToString(T Number)
{
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

inline bool str_contains(const std::string &str, const std::string &substr)
{
    return str.find(substr) != static_cast< size_t >(-1);
}

inline bool str_equal(const std::string &str, const std::string &substr)
{
    return str == substr;
}

using MurmurHashType = unsigned int;
MurmurHashType MurmurHash2(const void *key, int len, unsigned int seed = 0);

double getWallTime();
double getCpuTime();

std::pair< char *, long > readBinaryFile(const char *fname);
std::pair< char *, long > readFile(const char *fname, const char *mode);

void writeBinaryFile(const char *fname, const void *data, size_t type_size,
                     size_t length);

class Profiler
{
    bool _verbal;
    bool _started;
    double _startTime;

public:
    Profiler(bool verbal = true);
    void start();
    void step(const std::string &eventName = std::string());
    void finish(const std::string &eventName);
};

std::vector< char > strToVChar(const std::string &str);

char osSeparator();

long long file_size(const char *fname);

std::string makeIndents(int indent, int extra_spaces = 0);

std::vector< std::string > split(const std::string &str,
                                 const std::string &delim);

#endif // LAZYRT_HELP_FUNCTIONS_HPP
