#include "types/splitted_path.hpp"
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

SplittedPath::SplittedPath(const std::__cxx11::string &path)
    : _string(path), _isCalculatedSplittedPath(false)
{

}

const std::list<HashedFileName> &SplittedPath::splittedPath() const
{
    if (!_isCalculatedSplittedPath)
        calculateSplittedPath();
    return _splittedPath;
}

void SplittedPath::calculateSplittedPath() const
{
    MY_ASSERT(!_isCalculatedSplittedPath);
    static const char sep = osSeparator();

    size_t pos = 0, sepPos;
    while ((sepPos = _string.find(sep, pos)) != (size_t)-1) {
        if (pos != sepPos) {
            _splittedPath.push_back(_string.substr(pos, sepPos - pos));
        }

        pos = sepPos + 1;
    }

    _splittedPath.push_back(_string.substr(pos));

    _isCalculatedSplittedPath = true;
}

SplittedPath SplittedPath::operator+(const SplittedPath &extra_path) const
{
    SplittedPath concatenatedResult(_string + osSeparator() + extra_path.string());

    return concatenatedResult;
}



//std::__cxx11::list::iterator SplittedPath::find(const SplittedPath &root) const
//{
//    auto &rootFileNames = root.splittedPath();
//    auto &thisFileNames = splittedPath();
//    auto rootIter = rootFileNames.cbegin();
//    auto thisIter = thisFileNames.cbegin();

//    while ((rootIter != rootFileNames.cend())
//           && (thisIter != thisFileNames.cend())) {


//        ++rootIter;
//        ++thisIter;
//    }

//}

HashedFileName::HashedFileName(const std::__cxx11::string &str)
    : HashedString(str)
{

}
