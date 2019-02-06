#include "types/splitted_string.hpp"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#define STR_DOT (".")
#define STR_DOT_DOT ("..")

const MurmurHashType HashedFileName::_hashDot =
    MurmurHash2(STR_DOT, strlen(STR_DOT));
const MurmurHashType HashedFileName::_hashDotDot =
    MurmurHash2(STR_DOT_DOT, strlen(STR_DOT_DOT));

const char *get_home_dir()
{
    const char *homedir = NULL;
#ifdef _WIN32
    if ((homedir = getenv("HOME")) == NULL) {
        static char path[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) != S_OK) {
            MY_ASSERT(false);
            return NULL;
        }
        homedir = path;
    }
#else
    if ((homedir = getenv("HOME")) == NULL)
        homedir = getpwuid(getuid())->pw_dir;
#endif
    return homedir;
}

HashedString::HashedString(const std::string &str) : std::string(str), _hash(0)
{
}

MurmurHashType HashedString::hash() const
{
    if (_hash == 0)
        _hash = MurmurHash2(c_str(), length());

    return _hash;
}

HashedFileName::HashedFileName(const std::string &str) : HashedString(str) {}

static SplittedPath return_and_set_error(bool *error)
{
    MY_ASSERT(error != nullptr);
    if (error != nullptr)
        *error = true;
    return SplittedPath();
}

SplittedPath relative_path(const SplittedPath &path_to_file,
                           const SplittedPath &base, bool *error)
{
    const auto &splitted_file = path_to_file.splitted();
    const auto &splitted_base = base.splitted();

    auto it_file = splitted_file.cbegin();
    auto it_file_end = splitted_file.cend();

    auto it_base = splitted_base.cbegin();
    auto it_base_end = splitted_base.cend();

    if (splitted_base.size() > splitted_file.size())
        return return_and_set_error(error);

    SplittedPath result;
    result.setSeparator(base.separator());

    while (it_base != it_base_end) {
        if (*it_base == *it_file) {
            ++it_base;
            ++it_file;
            continue;
        }
        return return_and_set_error(error);
    }
    while (it_file != it_file_end) {
        result.append(*it_file);
        ++it_file;
    }
    return result;
}

SplittedPath absolute_path(const SplittedPath &path, const SplittedPath &base)
{
    if (!path.isRelative())
        return path;
    return base + path;
}

template < typename THashedString >
SplittedString< THashedString >::SplittedString()
{
    init();
}

template < typename THashedString >
SplittedString< THashedString >::SplittedString(const std::string &joint_,
                                                const std::string &separator_)
    : _joint(joint_), _separator(separator_)
{
    init();
}

template < typename THashedString >
SplittedString< THashedString >::SplittedString(
    const SplittedString::SplittedType &splitted_)
    : _splitted(splitted_)
{
    init();
}

template < typename THashedString >
void SplittedString< THashedString >::prepend(const THashedString &s)
{
    if (_isJointValid)
        clearJoint();

    if (!_isSplittedValid)
        _isSplittedValid = true;

    _splitted.insert(_splitted.begin(), s);
}

template < typename THashedString >
void SplittedString< THashedString >::append(const THashedString &s)
{
    if (!_isSplittedValid)
        split();
    clearJoint();

    if (_splitted.size() == 1 && _splitted.front().empty())
        _splitted.clear();
    _splitted.push_back(s);
}

template < typename THashedString >
void SplittedString< THashedString >::appendPath(
    const SplittedString< THashedString > &extra_path)
{
    MY_ASSERT(_separator == extra_path._separator);
    if (joint().empty())
        _joint = extra_path.joint();
    else if (!extra_path.joint().empty())
        _joint += _separator + extra_path._joint;
    clearSplitted();
}

template < typename THashedString >
bool SplittedString< THashedString >::empty() const
{
    return !_isSplittedValid && !_isJointValid;
}

template < typename THashedString >
const std::vector< THashedString > &
SplittedString< THashedString >::splitted() const
{
    if (!_isSplittedValid)
        split();
    return _splitted;
}

template < typename THashedString >
const std::string &SplittedString< THashedString >::joint() const
{
    if (!_isJointValid)
        join();
    return _joint;
}

template < typename THashedString >
const THashedString &SplittedString< THashedString >::last() const
{
    if (!_isSplittedValid)
        split();
    if (_splitted.empty())
        return emptyString();
    return _splitted.back();
}

template < typename THashedString >
void SplittedString< THashedString >::setSeparator(const std::string &sep)
{
    if (_separator == sep)
        return;
    if (_isJointValid && !_isSplittedValid)
        split();
    _separator = sep;
    _separatorSize = _separator.size();
    clearJoint();
}

template < typename THashedString >
void SplittedString< THashedString >::clear()
{
    clearJoint();
    clearSplitted();
}

template < typename THashedString >
void SplittedString< THashedString >::removeLast()
{
    if (splitted().empty())
        return;
    _splitted.pop_back();
    clearJoint();
}

template < typename THashedString >
void SplittedString< THashedString >::removeFirst()
{
    if (splitted().empty())
        return;
    _splitted.erase(_splitted.begin());
    clearJoint();
}

template < typename THashedString >
bool SplittedString< THashedString >::isRelative() const
{
    if (empty())
        return true;
    return !splitted().front().empty();
}

template < typename THashedString >
SplittedString< THashedString > SplittedString< THashedString >::
operator+(const THashedString &str)
{
    auto tmp = *this;
    tmp.append(str);
    return tmp;
}

template < typename THashedString >
bool SplittedString< THashedString >::
operator==(const SplittedString< THashedString > &other) const
{
    if (splitted().size() != other.splitted().size())
        return false;
    auto it = _splitted.cbegin();
    auto it2 = other._splitted.cbegin();
    for (; it != _splitted.cend(); ++it, ++it2) {
        if (*it != *it2)
            return false;
    }
    return true;
}

template < typename THashedString >
bool SplittedString< THashedString >::
operator!=(const SplittedString< THashedString > &other) const
{
    return !(*this == other);
}

template < typename THashedString >
bool SplittedString< THashedString >::
operator<(const SplittedString< THashedString > &other) const
{
    return joint() < other.joint();
}

template < typename THashedString >
std::string
SplittedString< THashedString >::jointSep(const std::string &separator) const
{
    auto tmp = *this;
    tmp.setSeparator(separator);
    return tmp.joint();
}

template < typename THashedString >
const std::string &SplittedString< THashedString >::namespaceSep()
{
    static std::string sep("::");
    return sep;
}

template < typename THashedString >
const std::string &SplittedString< THashedString >::osSep()
{
    static const char osSep = osSeparator();
    static std::string sep(std::string(&osSep, 1));
    return sep;
}

template < typename THashedString >
const std::string &SplittedString< THashedString >::unixSep()
{
    static std::string sep("/");
    return sep;
}

template < typename THashedString >
const THashedString &SplittedString< THashedString >::emptyString()
{
    static const THashedString _e = THashedString(std::string());
    return _e;
}

template < typename THashedString >
void SplittedString< THashedString >::clearJoint()
{
    if (_isJointValid) {
        _isJointValid = false;
        _joint.clear();
    }
}

template < typename THashedString >
void SplittedString< THashedString >::join() const
{
    MY_ASSERT(!_isJointValid);
    size_t totalSize = 0;
    if (!_splitted.empty()) {
        for (THashedString &sw : _splitted)
            totalSize += sw.size() + _separatorSize;
        totalSize -= _separatorSize;

        _joint.reserve(totalSize);
        bool first = true;
        for (THashedString &sw : _splitted) {
            if (!first)
                _joint += _separator;
            else
                first = false;
            _joint += sw;
        }
    }

    _isJointValid = true;
}

template < typename THashedString >
void SplittedString< THashedString >::clearSplitted()
{
    if (_isSplittedValid) {
        _isSplittedValid = false;
        _splitted.clear();
    }
}

template < typename THashedString >
void SplittedString< THashedString >::split() const
{
    MY_ASSERT(!_isSplittedValid);
    _isSplittedValid = true;
    if (_joint.empty())
        return;
    size_t pos = 0, sepPos;
    while ((sepPos = _joint.find(_separator, pos)) !=
           static_cast< size_t >(-1)) {
        _splitted.push_back(_joint.substr(pos, sepPos - pos));

        pos = sepPos + _separatorSize;
    }

    std::string lastItem = _joint.substr(pos);
    if (!lastItem.empty())
        _splitted.push_back(lastItem);
}

template < typename THashedString >
void SplittedString< THashedString >::init()
{
    if (_separator.empty())
        _separator = std::string("/");

    _separatorSize = _separator.size();

    normalize();

    _isJointValid = !_joint.empty();
    _isSplittedValid = !_splitted.empty();
}

template < typename THashedString >
void SplittedString< THashedString >::normalize()
{
    removeHomeDirSign();
    removeTrailingSeparators();
}

template < typename THashedString >
void SplittedString< THashedString >::removeHomeDirSign()
{
    size_t pos;
    while ((pos = _joint.find('~')) != static_cast< size_t >(-1)) {
        _joint.replace(pos, 1, std::string(get_home_dir()));
    }
}

template < typename THashedString >
void SplittedString< THashedString >::removeTrailingSeparators()
{
    while (_joint.find(_separator, _joint.size() - _separator.size()) !=
           static_cast< size_t >(-1)) {
        _joint.resize(_joint.size() - _separator.size());
    }
}

template < typename THashedString >
SplittedString< THashedString > SplittedString< THashedString >::
operator+(const SplittedString &extra_path) const
{
    SplittedString tmp = *this;
    tmp.appendPath(extra_path);
    return tmp;
}
