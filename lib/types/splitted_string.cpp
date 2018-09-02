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
