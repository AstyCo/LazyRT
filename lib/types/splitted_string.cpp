#include "types/splitted_string.hpp"

#include <string.h>

#define STR_DOT (".")
#define STR_DOT_DOT ("..")

const MurmurHashType HashedFileName::_hashDot = MurmurHash2(STR_DOT, strlen(STR_DOT));
const MurmurHashType HashedFileName::_hashDotDot = MurmurHash2(STR_DOT_DOT, strlen(STR_DOT_DOT));


HashedString::HashedString(const std::__cxx11::string &str)
    : std::string(str),
      _hash(0)
{

}

MurmurHashType HashedString::hash() const
{
    if (_hash == 0)
        _hash = MurmurHash2(c_str(), length());

    return _hash;
}

HashedFileName::HashedFileName(const std::__cxx11::string &str)
    : HashedString(str)
{

}

SplittedPath my_relative(const SplittedPath &path_to_file, const SplittedPath &base)
{
    const auto &splitted_file = path_to_file.splitted();
    const auto &splitted_base = base.splitted();

    auto it_file = splitted_file.cbegin();
    auto it_file_end = splitted_file.cend();


    auto it_base = splitted_base.cbegin();
    auto it_base_end = splitted_base.cend();

    MY_ASSERT(splitted_base.size() <= splitted_file.size());

    SplittedPath result;
    result.setOsSeparator();
    while (it_base != it_base_end) {
        if (*it_base == *it_file) {
            ++it_base;
            ++it_file;
            continue;
        }
        MY_ASSERT(false);
        result.append(std::string("."));
        return result;
    }
    while (it_file != it_file_end) {
        result.append(*it_file);
        ++it_file;
    }
    return result;
}
